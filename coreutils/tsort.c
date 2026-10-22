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

#समावेश "libbb.h"
#समावेश "common_bufsiz.h"

काष्ठा गाँठ {
	अचिन्हित in_गिनती;
	अचिन्हित out_गिनती;
	काष्ठा गाँठ **out;
	अक्षर name[1];
};

काष्ठा globals {
	काष्ठा गाँठ **nodes;
	अचिन्हित nodes_len;
};
#घोषणा G (*(काष्ठा globals*)bb_common_bufsiz1)
#घोषणा INIT_G() करो { \
	setup_common_bufsiz(); \
	BUILD_BUG_ON(माप(G) > COMMON_BUFSIZE); \
	G.nodes = शून्य; \
	G.nodes_len = 0; \
} जबतक (0)

अटल काष्ठा गाँठ *
get_गाँठ(स्थिर अक्षर *name)
{
	काष्ठा गाँठ *n;
	अचिन्हित a = 0;
	अचिन्हित b = G.nodes_len;

	/* Binary search for name */
	जबतक (a != b) {
		अचिन्हित m = (a + b) / 2;
		पूर्णांक cmp = म_भेद(name, G.nodes[m]->name);
		अगर (cmp == 0)
			वापस G.nodes[m]; /* found */
		अगर (cmp < 0) {
			b = m;
		} अन्यथा {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(माप(*n) + म_माप(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	म_नकल(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * माप(*G.nodes));
	स_हटाओ(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * माप(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	वापस n;
}

अटल व्योम
add_edge(काष्ठा गाँठ *a, काष्ठा गाँठ *b)
{
	a->out = xrealloc_vector(a->out, 6, a->out_गिनती);
	a->out[a->out_गिनती++] = b;
	b->in_गिनती++;
}

पूर्णांक tsort_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक tsort_main(पूर्णांक argc UNUSED_PARAM, अक्षर **argv)
{
	अक्षर *पंक्ति;
	माप_प्रकार linesz;
	ssize_t len;
	काष्ठा गाँठ *a;
	पूर्णांक cycles;
	अचिन्हित i;
#अगर ENABLE_FEATURE_CLEAN_UP
	अचिन्हित max_len;
#पूर्ण_अगर

	INIT_G();

	अगर (argv[1]) {
		अगर (argv[2])
			bb_show_usage();
		अगर (NOT_LONE_DASH(argv[1])) {
			बंद(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = शून्य;
	पंक्ति = शून्य;
	linesz = 0;
	जबतक ((len = getline(&पंक्ति, &linesz, मानक_निवेश)) != -1) {
		अक्षर *s = पंक्ति;
		जबतक (*(s = skip_whitespace(s)) != '\0') {
			काष्ठा गाँठ *b;
			अक्षर *word;

			word = s;
			s = skip_non_whitespace(s);
			अगर (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_गाँठ(word);
			अगर (!a) {
				a = b;
			} अन्यथा {
				अगर (a != b)
					add_edge(a, b);
				a = शून्य;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	अगर (a)
		bb_simple_error_संदेश_and_die("odd input");
	मुक्त(पंक्ति);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	cycles = 0;
#अगर ENABLE_FEATURE_CLEAN_UP
	max_len = G.nodes_len;
#पूर्ण_अगर
	जबतक (G.nodes_len) {
		काष्ठा गाँठ *n;

		/* Search for first node with no incoming edges */
		क्रम (i = 0; i < G.nodes_len; i++) {
			अगर (!G.nodes[i]->in_गिनती)
				अवरोध;
		}
		अगर (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			cycles++;
			i = 0;
#अगर_अघोषित TINY
			bb_error_संदेश("cycle at %s", G.nodes[i]->name);
#पूर्ण_अगर
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#अगर ENABLE_FEATURE_CLEAN_UP
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#पूर्ण_अगर

		/* And remove its outgoing edges */
		क्रम (i = 0; i < n->out_गिनती; i++)
			n->out[i]->in_गिनती--;

		माला_दो(n->name);
	}
#अगर ENABLE_FEATURE_CLEAN_UP
	क्रम (i = 0; i < max_len; i++) {
		मुक्त(G.nodes[i]->out);
		मुक्त(G.nodes[i]);
	}
	मुक्त(G.nodes);
#पूर्ण_अगर

	fflush_stdout_and_exit(cycles ? 1 : 0);
}
