/* vi: set sw=4 ts=4: */
/*
 * tsort implementation for busybox
 *
 * public domain -- David Leonard, 2022
 */
//config:config TSORT
//config:	bool "tsort (2.6 kb)"
//config:	default y
//config:	help
//config:	tsort performs a topological sort.

//applet:IF_TSORT(APPLET(tsort, BB_DIR_USR_BIN, BB_SUID_DROP))

//kbuild:lib-$(CONFIG_TSORT) += tsort.o

/* BB_AUDIT SUSv3 compliant */
/* http://www.opengroup.org/onlinepubs/007904975/utilities/tsort.html */

//usage:#define tsort_trivial_usage
//usage:       "[FILE]"
//usage:#define tsort_full_usage "\n\n"
//usage:       "Topological sort"
//usage:#define tsort_example_usage
//usage:       "$ echo -e \"a b\\nb c\" | tsort\n"
//usage:       "a\n"
//usage:       "b\n"
//usage:       "c\n"

#ಅನ್ತರ್ಭಾವ "libbb.h"
#ಅನ್ತರ್ಭಾವ "common_bufsiz.h"

ರಚನೆ ಗಂಟು {
	ಚಿಹ್ನೆರಹಿತ ಒಳ_ಎಣಿಕೆ;
	ಚಿಹ್ನೆರಹಿತ ಹೊರಹೋಗು_ಎಣಿಕೆ;
	ರಚನೆ ಗಂಟು **ಹೊರಹೋಗು;
	ಅಕ್ಷರ name[1];
};

ರಚನೆ globals {
	ರಚನೆ ಗಂಟು **nodes;
	ಚಿಹ್ನೆರಹಿತ nodes_len;
};
#ವ್ಯಾಖ್ಯಾನಿಸು G (*(ರಚನೆ globals*)bb_common_bufsiz1)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಸಿದ್ಧತೆ_common_bufsiz(); \
	BUILD_BUG_ON(ಗಾತ್ರ(G) > COMMON_BUFSIZE); \
	G.nodes = ಖಾಲಿ; \
	G.nodes_len = 0; \
} ಆಗ (0)

ಸ್ಥಿರ ರಚನೆ ಗಂಟು *
get_ಗಂಟು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ ಗಂಟು *n;
	ಚಿಹ್ನೆರಹಿತ a = 0;
	ಚಿಹ್ನೆರಹಿತ b = G.nodes_len;

	/* Binary search for name */
	ಆಗ (a != b) {
		ಚಿಹ್ನೆರಹಿತ m = (a + b) / 2;
		ಪೂರ್ಣಾಂಕ cmp = strcmp(name, G.nodes[m]->name);
		ಆದರೆ (cmp == 0)
			ಹಿಂತಿರುಗಿ G.nodes[m]; /*ಸಿಕ್ಕಿತು*/
		ಆದರೆ (cmp < 0) {
			b = m;
		} ಇಲ್ಲದಿದ್ದರೆ {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(ಗಾತ್ರ(*n) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * ಗಾತ್ರ(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * ಗಾತ್ರ(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ಹಿಂತಿರುಗಿ n;
}

ಸ್ಥಿರ ಶೂನ್ಯ
ಕೂಡಿಸು_ಅಂಚು(ರಚನೆ ಗಂಟು *a, ರಚನೆ ಗಂಟು *b)
{
	a->ಹೊರಹೋಗು = xrealloc_ಸದಿಶ(a->ಹೊರಹೋಗು, 6, a->ಹೊರಹೋಗು_ಎಣಿಕೆ);
	a->ಹೊರಹೋಗು[a->ಹೊರಹೋಗು_ಎಣಿಕೆ++] = b;
	b->ಒಳ_ಎಣಿಕೆ++;
}

ಪೂರ್ಣಾಂಕ tsort_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ tsort_main(ಪೂರ್ಣಾಂಕ argc ಬಳಸದ_PARAM, ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *ಸಾಲು;
	ಗಾತ್ರ_t linesz;
	ssize_t len;
	ರಚನೆ ಗಂಟು *a;
	ಪೂರ್ಣಾಂಕ ಚಕ್ರಗಳು;
	ಚಿಹ್ನೆರಹಿತ i;
#ಆದರೆ ENABLE_FEATURE_CLEAN_ಮೇಲೆ
	ಚಿಹ್ನೆರಹಿತ max_len;
#ಆದರೆ_ಅಂತ್ಯ

	INIT_G();

	ಆದರೆ (argv[1]) {
		ಆದರೆ (argv[2])
			bb_show_usage();
		ಆದರೆ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ಖಾಲಿ;
	ಸಾಲು = ಖಾಲಿ;
	linesz = 0;
	ಆಗ ((len = getline(&ಸಾಲು, &linesz, stdin)) != -1) {
		ಅಕ್ಷರ *s = ಸಾಲು;
		ಆಗ (*(s = skip_whitespace(s)) != '\0') {
			ರಚನೆ ಗಂಟು *b;
			ಅಕ್ಷರ *ಶಬ್ದ;

			ಶಬ್ದ = s;
			s = skip_non_whitespace(s);
			ಆದರೆ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ಗಂಟು(ಶಬ್ದ);
			ಆದರೆ (!a) {
				a = b;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ (a != b)
					ಕೂಡಿಸು_ಅಂಚು(a, b);
				a = ಖಾಲಿ;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ಆದರೆ (a)
		bb_simple_error_ಸಂದೇಶ_and_die("odd input");
	ಬಿಡುಗಡೆ(ಸಾಲು);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	ಚಕ್ರಗಳು = 0;
#ಆದರೆ ENABLE_FEATURE_CLEAN_ಮೇಲೆ
	max_len = G.nodes_len;
#ಆದರೆ_ಅಂತ್ಯ
	ಆಗ (G.nodes_len) {
		ರಚನೆ ಗಂಟು *n;

		/* Search for first node with no incoming edges */
		ಗಾಗಿ (i = 0; i < G.nodes_len; i++) {
			ಆದರೆ (!G.nodes[i]->ಒಳ_ಎಣಿಕೆ)
				ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			ಚಕ್ರಗಳು++;
			i = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ TINY
			bb_error_ಸಂದೇಶ("cycle at %s", G.nodes[i]->name);
#ಆದರೆ_ಅಂತ್ಯ
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#ಆದರೆ ENABLE_FEATURE_CLEAN_ಮೇಲೆ
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#ಆದರೆ_ಅಂತ್ಯ

		/* And remove its outgoing edges */
		ಗಾಗಿ (i = 0; i < n->ಹೊರಹೋಗು_ಎಣಿಕೆ; i++)
			n->ಹೊರಹೋಗು[i]->ಒಳ_ಎಣಿಕೆ--;

		puts(n->name);
	}
#ಆದರೆ ENABLE_FEATURE_CLEAN_ಮೇಲೆ
	ಗಾಗಿ (i = 0; i < max_len; i++) {
		ಬಿಡುಗಡೆ(G.nodes[i]->ಹೊರಹೋಗು);
		ಬಿಡುಗಡೆ(G.nodes[i]);
	}
	ಬಿಡುಗಡೆ(G.nodes);
#ಆದರೆ_ಅಂತ್ಯ

	fflush_stdout_and_exit(ಚಕ್ರಗಳು ? 1 : 0);
}
