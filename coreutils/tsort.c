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

#अन्तर्भाव "libbb.h"
#अन्तर्भाव "common_bufsiz.h"

संरचना ग्रन्थिः {
	अचिह्नित अन्तर्_गणनम्;
	अचिह्नित बहिर्गमनम्_गणनम्;
	संरचना ग्रन्थिः **बहिर्गमनम्;
	वर्ण name[1];
};

संरचना globals {
	संरचना ग्रन्थिः **nodes;
	अचिह्नित nodes_len;
};
#परिभाषय G (*(संरचना globals*)bb_common_bufsiz1)
#परिभाषय INIT_G() कुरु { \
	सज्जता_common_bufsiz(); \
	BUILD_BUG_ON(परिमाण(G) > COMMON_BUFSIZE); \
	G.nodes = रिक्त; \
	G.nodes_len = 0; \
} यावत् (0)

स्थिर संरचना ग्रन्थिः *
get_ग्रन्थिः(ध्रुव वर्ण *name)
{
	संरचना ग्रन्थिः *n;
	अचिह्नित a = 0;
	अचिह्नित b = G.nodes_len;

	/* Binary search for name */
	यावत् (a != b) {
		अचिह्नित m = (a + b) / 2;
		पूर्णाङ्क cmp = वाक्यं_तुलय(name, G.nodes[m]->name);
		यदि (cmp == 0)
			प्रत्यागम G.nodes[m]; /*प्राप्तम्*/
		यदि (cmp < 0) {
			b = m;
		} अन्यथा {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(परिमाण(*n) + वाक्यदैर्घ्यम्(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	वाक्यं_प्रतिलिख(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * परिमाण(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * परिमाण(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	प्रत्यागम n;
}

स्थिर शून्य
सङ्कलय_edge(संरचना ग्रन्थिः *a, संरचना ग्रन्थिः *b)
{
	a->बहिर्गमनम् = xrealloc_सदिशम्(a->बहिर्गमनम्, 6, a->बहिर्गमनम्_गणनम्);
	a->बहिर्गमनम्[a->बहिर्गमनम्_गणनम्++] = b;
	b->अन्तर्_गणनम्++;
}

पूर्णाङ्क tsort_main(पूर्णाङ्क argc, वर्ण **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णाङ्क tsort_main(पूर्णाङ्क argc अप्रयुक्तम्_PARAM, वर्ण **argv)
{
	वर्ण *पङ्क्तिः;
	आकार_t linesz;
	ssize_t len;
	संरचना ग्रन्थिः *a;
	पूर्णाङ्क आवृत्तयः;
	अचिह्नित i;
#यदि ENABLE_FEATURE_CLEAN_ऊर्ध्वम्
	अचिह्नित max_len;
#यदि_अन्तः

	INIT_G();

	यदि (argv[1]) {
		यदि (argv[2])
			bb_show_usage();
		यदि (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = रिक्त;
	पङ्क्तिः = रिक्त;
	linesz = 0;
	यावत् ((len = getline(&पङ्क्तिः, &linesz, stdin)) != -1) {
		वर्ण *s = पङ्क्तिः;
		यावत् (*(s = skip_whitespace(s)) != '\0') {
			संरचना ग्रन्थिः *b;
			वर्ण *word;

			word = s;
			s = skip_non_whitespace(s);
			यदि (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ग्रन्थिः(word);
			यदि (!a) {
				a = b;
			} अन्यथा {
				यदि (a != b)
					सङ्कलय_edge(a, b);
				a = रिक्त;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	यदि (a)
		bb_simple_error_सन्देशः_and_die("odd input");
	मुक्त(पङ्क्तिः);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	आवृत्तयः = 0;
#यदि ENABLE_FEATURE_CLEAN_ऊर्ध्वम्
	max_len = G.nodes_len;
#यदि_अन्तः
	यावत् (G.nodes_len) {
		संरचना ग्रन्थिः *n;

		/* Search for first node with no incoming edges */
		प्रति (i = 0; i < G.nodes_len; i++) {
			यदि (!G.nodes[i]->अन्तर्_गणनम्)
				भङ्ग;
		}
		यदि (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			आवृत्तयः++;
			i = 0;
#यदि_अपरिभाषितम् TINY
			bb_error_सन्देशः("cycle at %s", G.nodes[i]->name);
#यदि_अन्तः
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#यदि ENABLE_FEATURE_CLEAN_ऊर्ध्वम्
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#यदि_अन्तः

		/* And remove its outgoing edges */
		प्रति (i = 0; i < n->बहिर्गमनम्_गणनम्; i++)
			n->बहिर्गमनम्[i]->अन्तर्_गणनम्--;

		पङ्क्तिं_लिख(n->name);
	}
#यदि ENABLE_FEATURE_CLEAN_ऊर्ध्वम्
	प्रति (i = 0; i < max_len; i++) {
		मुक्त(G.nodes[i]->बहिर्गमनम्);
		मुक्त(G.nodes[i]);
	}
	मुक्त(G.nodes);
#यदि_अन्तः

	fflush_stdout_and_exit(आवृत्तयः ? 1 : 0);
}
