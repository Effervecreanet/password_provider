#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <locale.h>


#define STRING_LENGTH	0xff
#define PIN_LENGTH	4


#define PUNCTSET_MAX_INPUT 23
#define BACKSLASH '\\'


#define DIGIT 	     0001	/* At least one digit must be in the final
				 * computed password */

#define ALPHA_UPPER  0002	/* At least one upper-case letter must be in
				 * the final computed password */

#define ALPHA_LOWER  0004	/* At least one lower-case letter must be in
				 * the final computed password */

#define PUNCT        0010	/* At least one punctuation character must be
				 * in the final computed password */

#define PUNCTSET     0020	/* At least one user input punctuation
				 * character must be in the final computed
				 * password */

#define ALPHA     	  (ALPHA_UPPER | ALPHA_LOWER)	/* Alphabetic password
							 * with lower-case
							 * letters and
							 * upper-case letters */

#define ALPHA_NUM 	  (ALPHA | DIGIT)	/* Alphanumeric password with
						 * lower-case letters,
						 * upper-case letters and
						 * digits */

#define ALPHA_NUM_PUNCT   (ALPHA_NUM | PUNCT)	/* ASCII printable characters
						 * password, the strongest */

#define ALPHA_NUM_PUNCTSET (ALPHA | DIGIT | PUNCTSET)	/* ASCII printable
							 * characters password
							 * with spectified
							 * punctuation
							 * characters */

#define DEFAULT_PASSWORD_LENGTH 8	/* Default password are 7 characters
					 * and 1 nul-terminating '\0'
					 * character */


static int	password_length = DEFAULT_PASSWORD_LENGTH;


/* Prototyped functions */

unsigned int check_passwd(unsigned char *passwd, unsigned int password_type, char *punctset);
int gen_string(unsigned char *string, unsigned int password_type, char *punctset);
void help(void);
int verify_input_set(char *optarg);


unsigned int
check_passwd(unsigned char *passwd, unsigned int password_type, char *punctset)
{
	bool		alpha = false;
	bool		digit = false;
	bool		punct = false;
	int		ret = 0;

	switch (password_type) {
	case ALPHA_NUM:
		while (*passwd++) {
			if (isalpha(*passwd))
				alpha = true;
			if (isdigit(*passwd))
				digit = true;
		}
		if (alpha && digit)
			ret = 1;
		break;
	case ALPHA_NUM_PUNCT:
	case ALPHA_NUM_PUNCTSET:
		while (*passwd++) {
			if (isalpha(*passwd))
				alpha = true;
			if (isdigit(*passwd))
				digit = true;
			if (ispunct(*passwd))
				punct = true;
		}
		if (alpha && digit && punct)
			ret = 1;

		break;
	default:
		ret = 1;
		break;
	}

	return ret;
}

int
gen_string(unsigned char *string, unsigned int password_type, char *punctset)
{
	unsigned int	i;
	unsigned char	c[4];
	unsigned int	h;
	unsigned char  *string_sav = string;


	while (strlen((const char *)string_sav) < password_length) {
		rand_s(&i);

		c[0] = (unsigned char)i & 0x000000ff;
		i = i >> 8;
		c[1] = (unsigned char)i & 0x0000ff;
		i = i >> 8;
		c[2] = (unsigned char)i & 0x00ff;
		i = i >> 8;
		c[3] = (unsigned char)i & 0xff;

		for (h = 0; h < sizeof(c); h++) {
			switch (password_type) {
			case DIGIT:
				if (isdigit(c[h]))
					*string++ = c[h];
				break;
			case ALPHA:
				if (isalpha(c[h]))
					*string++ = c[h];
				break;
			case ALPHA_NUM:
			alphanum:	if (isalnum(c[h]))
				*string++ = c[h];
						break;
			case ALPHA_LOWER:
				if (islower(c[h]))
					*string++ = c[h];
				break;
			case ALPHA_NUM_PUNCT:
				if (isgraph(c[h]) && c[h] != BACKSLASH)
					*string++ = c[h];
				break;
			case ALPHA_NUM_PUNCTSET:
				if (ispunct(c[h])) {
					char           *p_ps = NULL;
					for (p_ps = punctset; *p_ps; p_ps++) {
						if (c[h] == *p_ps) {
							*string++ = c[h];
							break;
						}
					}
				}
				goto alphanum;
				break;
			default:
				break;
			}
		}
	}

	string = string_sav;

	/*
	 * Here we terminate the password string, note that the real computed
	 * password can be longer than the final password
	 */
	*(string + password_length) = '\0';

	return 1;
}

/* Print help */
void
help(void)
{
	system("chcp 65001 > nul");
	printf(
		"Usage: pp [-abdep] [-c longueur] [-s jeu de signes]\n\t"
		"-a génére un mot de passe alphabétique\n\t"
		"-b génére un mot de passe alphabétique avec des lettres minuscules\n\t"
		"-d génére un mot de passe numérique\n\t"
		"-e génére un mot de passe alphanumérique\n\t"
		"-p génére un mot de passe alphanumérique avec des signes de poncuations\n\t"
		"-s génére un mot de passe alphabétique avec des signes de ponctuations précisés\n\t"
		"-v à propos de ce programme\n\t"
		"-h vous redirige vers une aide plus complète\n");
}

/* Print help */
void
print_apropos(void)
{
	system("chcp 65001 > nul");
	printf("\r\nhttps://www.effervecrea.net - se créer son gestionnaire de mots de passe\r\n\r\n\t"
		"description\t:\tgénérateur et fournisseur de mots de passe faibles/forts\r\n\t"
		"nom\t\t:\tpp\r\n\t"
		"type\t\t:\texécutable console\r\n\t"
		"date\t\t:\tlundi 2 août 2021\r\n\t"
		"crédit\t\t:\tFranck Lesage\r\n\t"
		"e-mail:\t\t:\teffervecreanet@gmail.com\r\n\t");
}

/*
 * We check that the input punctset is'nt too long or empty or doesn't
 * contain digit and alpha characters
 */
int
verify_input_set(char *optarg)
{
	int		len = 0;
	char           *pr1;

	len = strlen(optarg);

	/* Check that set isn't too long */
	if (len > PUNCTSET_MAX_INPUT) {
		printf("Le jeu de ponctuation est trop long\n");
		return 0;
	}
	else if (len == 0) {
		/* Check that the set isn't empty */
		printf("Le jeu de ponctuation est vide\n");
		return 0;
	}
	/* Check that set is a punctation characters set */
	for (pr1 = optarg; *pr1; pr1++) {
		if (!ispunct(*pr1)) {
			printf("Le jeu de ponctuation a un caractère alphanumérique (%c)\n", *pr1);
			return 0;
		}
	}

	return 1;
}

/* main function, entry point of the password provider program */
int
main(int argc, char *argv[])
{
	unsigned char *password_string = NULL;
	char		ch;
	unsigned short	nbytes = 0;
	char           *outfile = NULL;
	int		len_punctset;
	int		len_output;
	unsigned int password_type = 0;
	char           *punctset = NULL;
	FILE           *output;
	int	argc_sav = argc;

	if (argc == 1) {
		help();
		exit(0);
	}

	output = NULL;

	/*
	 * Without options the password type is alpha-numeric with
	 * punctation, the strongest password type.
	 */
	argc = 1;
	while (argc < argc_sav) {
		if (*(argv[argc] + 1) == 'a') {
			password_type = ALPHA;
		}
		else if (*(argv[argc] + 1) == 'b') {
			password_type = ALPHA_LOWER;
		}
		else if (*(argv[argc] + 1) == 'c') {

			nbytes = atoi(argv[++argc]);

			if (nbytes == 0) {
				printf("La longueur est invalide\n");
				exit(0);
			}
			password_length = nbytes;
		}
		else if (*(argv[argc] + 1) == 'd') {
			password_type = DIGIT;
		}
		else if (*(argv[argc] + 1) == 'e') {
			password_type = ALPHA_NUM;
		}
		else if (*(argv[argc] + 1) == 'p') {
			password_type = ALPHA_NUM_PUNCT;
		}
		else if (*(argv[argc] + 1) == 's') {
			if (!verify_input_set(argv[++argc]))
				exit(0);
			len_punctset = strlen(argv[argc]);
			if ((punctset = malloc(len_punctset)) == NULL)
				/* err(EX_OSERR, NULL); */;
			strncpy(punctset, argv[argc], len_punctset);
			password_type = ALPHA_NUM_PUNCTSET;
		}
		else if (*(argv[argc] + 1) == 'h') {
			ShellExecute(NULL, "open",
				"https://www.effervecrea.net/generateur-de-mot-de-passe",
				NULL,
				NULL,
				SW_SHOWNORMAL);
			ShellExecute(NULL, "open",
				"https://www.effervecrea.net/se-creer-son-gestionnaire-de-mots-de-passe",
				NULL,
				NULL,
				SW_SHOWNORMAL);
			exit(0);
		}
		else if (*(argv[argc] + 1) == 'v') {
			print_apropos();
			exit(0);
		}
		else {
			password_type = ALPHA_NUM_PUNCT;
			break;
		}
		argc++;
	}


	if (!outfile) {
		output = stdout;
	}
	else {
		/* Open the user specified output filename */
		if ((output = fopen(outfile, "w")) == NULL)
			/* err(EX_CANTCREAT, "%s", outfile); */;
	}

	if ((password_string = malloc(password_length)) == NULL)
		/* err(EX_OSERR, NULL); */;

	do {
		memset(password_string, 0, password_length);
		/*
		 * We generate the password string according to specified
		 * condition(s)
		 */
		gen_string(password_string, password_type, punctset);
		/*
		 * Until the password respect the condition(s), we retry
		 */
	} while (check_passwd(password_string, password_type, punctset) != 1);

	/*
	 * Output the final password into standart output or the user
	 * specified file
	 */
	if (fprintf(output, "%s", password_string) < 0)
		/* err(EX_IOERR, "%s", outfile); */;

	/* Close the user specified file */
	if (fclose(output) != 0)
		/* err(EX_OSERR, "%s", outfile); */;

	if (punctset != NULL)
		free(punctset);

	free(password_string);

	return 1;
}
