#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
	char path[1024];
	char confdir[1024];
} ConfKey;

typedef struct {
	char items[16][1024];
	int count;
} StrList;

typedef struct {
	char label[1024];
	char shell[1024];

	StrList bins;

	char start[1024];

	StrList stop;
} ConfServiceKey;

static void trim(char *s) {
	size_t n = strlen(s);
	while (n && isspace((unsigned char)s[n - 1])) s[--n] = 0;
	char *p = s;
	while (isspace((unsigned char)*p)) p++;
	memmove(s, p, strlen(p) + 1);
}

static int parse_line(char *line, char **key, char **value) {
	trim(line);
	char *start=line;
	while(*start == ' ' || *start == '\t')
		start++;

	if(*start == '\0' || *start == '#')
		return -1;

	char *equal = strchr(start, '=');
	*equal = '\0';
	trim(start);
	*key = start;
	*value = equal + 1;

	return 0;
}

static const char *parse_string(const char *s, char *dst, size_t size) {
	size_t n = 0;

	if(*s == '"')
		s++;

	while(*s && *s != '"') {
		if(*s == '\\' && s[1] != '\0')
			s++;
		if(n < size - 1)
			dst[n++] = *s;
		s++;
	}
	dst[n] = '\0';
	
	if(*s == '"')
		s++;

	return s;
}

static void parse_list(const char*s, StrList *out) {
	out->count = 0;

	while(*s && out->count < 16) {
		while(*s == ',' || *s == ' ' || *s == '\t')
			s++;
		if(*s == '\0')
			break;

		char *dst = out->items[out->count];
		size_t n = 0;

		if(*s == '"') {
			s = parse_string(s, out->items[out->count], 1024);
		} else {
			while(*s && *s != ',' && n < 1023)
				dst[n++] = *s++;
			while(n > 0 && (dst[n-1] == ' ' || dst[n-1] == '\t'))
				n++;
			dst[n] = '\0';
		}

		out->count++;
	}
}

static void first_item(const StrList *l, char *dst, size_t size) {
	snprintf(dst, size, "%s", l->count > 0 ? l->items[0] : "");
}

static int load_root_config(const char *filename, ConfKey *config) {
	FILE *f = fopen(filename, "r");
	char line[4096];
	while (fgets(line, sizeof(line), f) != NULL) {
		char *key, *value;
		if (parse_line(line, &key, &value) != 0)
			continue;
		if(strcmp(key, "path") == 0) {
			StrList l;
			parse_list(value, &l);
			first_item(&l, config->path, sizeof(config->path));
		}
		if(strcmp(key, "confdir") == 0) {
			StrList l;
			parse_list(value, &l);
			first_item(&l, config->confdir, sizeof(config->confdir));
		}
	}
	fclose(f);
	return 0;
}

static int load_service_config(const char *confdir, const char *name, ConfServiceKey *svc) {
	char filename[1100];
	snprintf(filename, sizeof(filename), "%s/%s.omconf", confdir, name);

	FILE *f = fopen(filename, "r");
	char line[4096];
	while (fgets(line, sizeof(line), f) != NULL) {
		char *key, *value;
		if(parse_line(line, &key, &value) != 0)
			continue;

		StrList l;
		parse_list(value, &l);

		if (strcmp(key, "label") == 0)
			first_item(&l, svc->label, sizeof(svc->label));
		else if (strcmp(key, "shell") == 0)
			first_item(&l, svc->shell, sizeof(svc->shell));
		else if (strcmp(key, "bins") == 0)
			svc->bins = l;
		else if (strcmp(key, "start") == 0)
			first_item(&l, svc->start, sizeof(svc->start));
		else if (strcmp(key, "stop") == 0)
			svc->stop = l;
	}

	fclose(f);

	return 0;
}

/* attend que plus aucun processus ne tourne (au plus `seconds` secondes) */
static int list_pids(const StrList *bins, int *pids, int max) {
	int n = 0;
	for (int i = 0; i < bins->count; i++) {
		char cmd[1100];
		snprintf(cmd, sizeof cmd, "pgrep -x '%s'", bins->items[i]);
		FILE *p = popen(cmd, "r");
		if (!p) continue;
		while (n < max && fscanf(p, "%d", &pids[n]) == 1)
			n++;
		pclose(p);
	}
	return n;
}

static int wait_gone(const StrList *bins, int seconds) {
	int pids[256];
	for (int i = 0; i < seconds; i++) {
		if (list_pids(bins, pids, 256) == 0) return 1;
		sleep(1);
	}
	return list_pids(bins, pids, 256) == 0;
}

static void start(const ConfServiceKey *svc) {
	if (!svc->start[0]) {
		printf("%s : pas de commande de démarrage\n", svc->label);
		return;
	}

	int pids[256];
	if (list_pids(&svc->bins, pids, 256) > 0) {
		printf("%s : déjà actif (pid %d)\n", svc->label, pids[0]);
		return;
	}

	/* le shell expanse lui-même $USER, $HOME... ; nohup + & = détaché */
	char cmd[1100];
	snprintf(cmd, sizeof cmd, "nohup %s >/dev/null 2>&1 &", svc->start);
	printf("  $ %s\n", cmd);
	system(cmd);

	sleep(1);
	if (list_pids(&svc->bins, pids, 256) > 0)
		printf("%s : lancé (pid %d)\n", svc->label, pids[0]);
	else
		printf("%s : lancé mais aucun processus détecté\n", svc->label);
}

static void stop(const ConfServiceKey *svc) {
	if (svc->stop.count == 0) {
		printf("%s : pas de commande d'arrêt\n", svc->label);
		return;
	}

	for (int i = 0; i < svc->stop.count; i++) {
		printf("  $ %s\n", svc->stop.items[i]);
		system(svc->stop.items[i]);
	}

	/* Filet de sécurité : encore actif après 3 s => SIGTERM, puis 15 s d'attente */
	if (svc->bins.count > 0 && !wait_gone(&svc->bins, 3)) {
		for (int i = 0; i < svc->bins.count; i++) {
			char cmd[1100];
			snprintf(cmd, sizeof cmd, "pkill -TERM -x '%s'", svc->bins.items[i]);
			printf("  $ %s\n", cmd);
			system(cmd);
		}
		if (!wait_gone(&svc->bins, 15)) {
			printf("%s : toujours actif après 15 s\n", svc->label);
			return;
		}
	}
	printf("%s : arrêté\n", svc->label);
}
static void status(const ConfServiceKey *svc) {
	int pids[256];
	int n = list_pids(&svc->bins, pids, 256);
	if (n == 0) {
		printf("%s : arrêté\n", svc->label);
		return;
	}
	printf("%s : actif (pid", svc->label);
	for (int i = 0; i < n; i++) printf(" %d", pids[i]);
	printf(")\n");
}

int main(int argc, char **argv) {
	if (
		argc < 4 ||
		(strcmp(argv[2], "start") && strcmp(argv[2], "stop") && strcmp(argv[2], "status"))
	) {
		printf("Usage : service services.json start|stop service1 [service2 ...]\n");
		return 1;
	}

	ConfKey config = {0};
	load_root_config(argv[1], &config);


	printf("config file: %s\n", argv[1]);
	printf("Action: %s\n", argv[2]);
	printf("Path_root: %s\n", config.path);
	printf("config_dir: %s\n", config.confdir);

	for (int i = 3; i < argc; i++) {
		ConfServiceKey svc = {0};
		load_service_config(config.confdir, argv[i], &svc);

		printf("[%d]: %s.omconf : %s\n", i - 3, argv[i], svc.label);
		printf("  shell: %s\n", svc.shell);
		printf("  start: %s\n", svc.start);

		for (int j = 0; j < svc.bins.count; j++)
			printf("  bins[%d]: %s\n", j, svc.bins.items[j]);
		for (int j = 0; j < svc.stop.count; j++)
			printf("  stop[%d]: %s\n", j, svc.stop.items[j]);

		if (!strcmp(argv[2], "start")) start(&svc);
		else if (!strcmp(argv[2], "stop")) stop(&svc);
		else if (!strcmp(argv[2], "status")) status(&svc);
	}

	return 0;
}