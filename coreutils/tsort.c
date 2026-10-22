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

#ਅਨ੍ਤਰ੍ਭਾਵ "libbb.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "common_bufsiz.h"

ਬਣਤਰ ਗੰਢ {
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਵਿੱਚ_ਗਿਣਤੀ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਾਹਰਲਾ_ਗਿਣਤੀ;
	ਬਣਤਰ ਗੰਢ **ਬਾਹਰਲਾ;
	ਅੱਖਰ name[1];
};

ਬਣਤਰ globals {
	ਬਣਤਰ ਗੰਢ **nodes;
	ਬਿਨਾਨਿਸ਼ਾਨੀ nodes_len;
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*(ਬਣਤਰ globals*)bb_common_bufsiz1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਤਿਆਰੀ_common_bufsiz(); \
	BUILD_BUG_ON(ਆਕਾਰ(G) > COMMON_BUFSIZE); \
	G.nodes = ਨਿਰਾ; \
	G.nodes_len = 0; \
} ਜਦੋਂ (0)

ਸਥਿਰ ਬਣਤਰ ਗੰਢ *
get_ਗੰਢ(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ ਗੰਢ *n;
	ਬਿਨਾਨਿਸ਼ਾਨੀ a = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ b = G.nodes_len;

	/* Binary search for name */
	ਜਦੋਂ (a != b) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ m = (a + b) / 2;
		ਪੂਰਨ cmp = strcmp(name, G.nodes[m]->name);
		ਜੇ (cmp == 0)
			ਵਾਪਸ G.nodes[m]; /*ਮਿਲਿਆ*/
		ਜੇ (cmp < 0) {
			b = m;
		} ਨਹੀਂਤਾਂ {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(ਆਕਾਰ(*n) + ਲੜੀ_ਲੰਬਾਈ(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * ਆਕਾਰ(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * ਆਕਾਰ(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	ਵਾਪਸ n;
}

ਸਥਿਰ ਖਾਲੀ
ਜੋੜੋ_ਕਿਨਾਰਾ(ਬਣਤਰ ਗੰਢ *a, ਬਣਤਰ ਗੰਢ *b)
{
	a->ਬਾਹਰਲਾ = xrealloc_ਸਦਿਸ਼(a->ਬਾਹਰਲਾ, 6, a->ਬਾਹਰਲਾ_ਗਿਣਤੀ);
	a->ਬਾਹਰਲਾ[a->ਬਾਹਰਲਾ_ਗਿਣਤੀ++] = b;
	b->ਵਿੱਚ_ਗਿਣਤੀ++;
}

ਪੂਰਨ tsort_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ tsort_main(ਪੂਰਨ argc ਅਣਵਰਤਿਆ_PARAM, ਅੱਖਰ **argv)
{
	ਅੱਖਰ *ਸਤਰ;
	ਮਾਪ_t linesz;
	ssize_t len;
	ਬਣਤਰ ਗੰਢ *a;
	ਪੂਰਨ ਚੱਕਰਾਂ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ i;
#ਜੇ ENABLE_FEATURE_CLEAN_ਉੱਪਰ
	ਬਿਨਾਨਿਸ਼ਾਨੀ max_len;
#ਜੇ_ਅੰਤ

	INIT_G();

	ਜੇ (argv[1]) {
		ਜੇ (argv[2])
			bb_show_usage();
		ਜੇ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ਨਿਰਾ;
	ਸਤਰ = ਨਿਰਾ;
	linesz = 0;
	ਜਦੋਂ ((len = getline(&ਸਤਰ, &linesz, stdin)) != -1) {
		ਅੱਖਰ *s = ਸਤਰ;
		ਜਦੋਂ (*(s = skip_whitespace(s)) != '\0') {
			ਬਣਤਰ ਗੰਢ *b;
			ਅੱਖਰ *ਸ਼ਬਦ_2;

			ਸ਼ਬਦ_2 = s;
			s = skip_non_whitespace(s);
			ਜੇ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ਗੰਢ(ਸ਼ਬਦ_2);
			ਜੇ (!a) {
				a = b;
			} ਨਹੀਂਤਾਂ {
				ਜੇ (a != b)
					ਜੋੜੋ_ਕਿਨਾਰਾ(a, b);
				a = ਨਿਰਾ;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ਜੇ (a)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("odd input");
	ਮੁਕਤ(ਸਤਰ);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	ਚੱਕਰਾਂ = 0;
#ਜੇ ENABLE_FEATURE_CLEAN_ਉੱਪਰ
	max_len = G.nodes_len;
#ਜੇ_ਅੰਤ
	ਜਦੋਂ (G.nodes_len) {
		ਬਣਤਰ ਗੰਢ *n;

		/* Search for first node with no incoming edges */
		ਲਈ (i = 0; i < G.nodes_len; i++) {
			ਜੇ (!G.nodes[i]->ਵਿੱਚ_ਗਿਣਤੀ)
				ਰੋਕੋ;
		}
		ਜੇ (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			ਚੱਕਰਾਂ++;
			i = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ TINY
			bb_error_ਸੁਨੇਹਾ("cycle at %s", G.nodes[i]->name);
#ਜੇ_ਅੰਤ
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#ਜੇ ENABLE_FEATURE_CLEAN_ਉੱਪਰ
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#ਜੇ_ਅੰਤ

		/* And remove its outgoing edges */
		ਲਈ (i = 0; i < n->ਬਾਹਰਲਾ_ਗਿਣਤੀ; i++)
			n->ਬਾਹਰਲਾ[i]->ਵਿੱਚ_ਗਿਣਤੀ--;

		puts(n->name);
	}
#ਜੇ ENABLE_FEATURE_CLEAN_ਉੱਪਰ
	ਲਈ (i = 0; i < max_len; i++) {
		ਮੁਕਤ(G.nodes[i]->ਬਾਹਰਲਾ);
		ਮੁਕਤ(G.nodes[i]);
	}
	ਮੁਕਤ(G.nodes);
#ਜੇ_ਅੰਤ

	fflush_stdout_and_exit(ਚੱਕਰਾਂ ? 1 : 0);
}
