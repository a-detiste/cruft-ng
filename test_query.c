#define LIBDPKG_VOLATILE_API
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/db-fsys.h>
#include <dpkg/pkg-list.h>

int main()
{
	dpkg_fsys_set_dir("");
	dpkg_program_init("test_query");
	modstatdb_open(msdbrw_readonly);
	ensure_allinstfiles_available_quiet();
	ensure_diversions();
	struct fsys_namenode *namenode;

	int fd[2];
	if (pipe(fd) != 0) {
		perror("pipe");
		exit(1);
	}
	if(!fork()) // child
	{
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execl("./test_query.sh", NULL);
		printf("Failed to execute command test_query.sh: %s\n", strerror(errno));
		exit(1);
	}

	close(fd[1]);
	FILE* fp;
	fp=fdopen(fd[0], "r");
	char buf[4096];
	while (fgets(buf, sizeof(buf),fp))
	{
		buf[strlen(buf)-1] = '\0'; // remove '/n'
		printf("path: %s\n", buf);
		namenode = fsys_hash_find_node(buf, FHFF_NO_COPY);
		if (namenode->divert) {
		        printf("package: %s\n", namenode->divert->pkgset->name);
		} else if(namenode->packages) {
			printf("package: %s\n", namenode->packages->pkg->set->name);
		}
	}
	fclose(fp);
	modstatdb_shutdown();
	dpkg_program_done();
}
