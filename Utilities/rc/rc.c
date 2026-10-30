#define _POSIX_C_SOURCE 200809L /* for execlp under -std=c99 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SSH_USER     "gwitske"
#define SSH_DOMAIN   ".cs.siue.edu"
#define DEFAULT_HOST "home"
#define MAX_HOST_LEN 253

static void usage(FILE *out, const char *prog)
{
	fprintf(out,
		"Usage: %s [host]\n"
		"  Connects with: ssh " SSH_USER "@<host>" SSH_DOMAIN "\n"
		"  host defaults to \"" DEFAULT_HOST "\". Aliases: rad -> radish\n",
		prog);
}

/* Accept only characters that are valid in a hostname, so the argument
 * can never be interpreted as anything other than a host. */
static int valid_host(const char *host)
{
	size_t len = strlen(host);

	if (len == 0 || len > MAX_HOST_LEN)
		return 0;
	if (host[0] == '-' || host[0] == '.' || host[len - 1] == '-' || host[len - 1] == '.')
		return 0;
	if (strstr(host, "..") != NULL)
		return 0;
	for (size_t i = 0; i < len; i++)
	{
		unsigned char c = (unsigned char)host[i];
		if (!isalnum(c) && c != '-' && c != '.')
			return 0;
	}
	return 1;
}

int main(int argc, char *argv[])
{
	const char *prog = (argc > 0 && argv[0] != NULL) ? argv[0] : "rc";
	const char *host = DEFAULT_HOST;
	char dest[sizeof(SSH_USER) + MAX_HOST_LEN + sizeof(SSH_DOMAIN) + 2];

	if (argc > 2)
	{
		fprintf(stderr, "Error: too many arguments (expected at most 1 host)\n");
		usage(stderr, prog);
		return 1;
	}

	if (argc == 2)
	{
		if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			usage(stdout, prog);
			return 0;
		}
		host = argv[1];
	}

	if (strcmp(host, "rad") == 0)
		host = "radish";

	if (!valid_host(host))
	{
		fprintf(stderr, "Error: invalid host name \"%s\"\n", host);
		usage(stderr, prog);
		return 1;
	}

	int n = snprintf(dest, sizeof(dest), SSH_USER "@%s" SSH_DOMAIN, host);
	if (n < 0 || (size_t)n >= sizeof(dest))
	{
		fprintf(stderr, "Error: host name too long\n");
		return 1;
	}

	/* Replace this process with ssh directly (no shell), so ssh's exit
	 * status becomes ours and no shell metacharacters are interpreted. */
	execlp("ssh", "ssh", dest, (char *)NULL);
	perror("Error: could not run ssh");
	return 127;
}
