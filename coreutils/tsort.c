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

#ضمن "libbb.h"
#ضمن "common_bufsiz.h"

بنية گرہ {
	بنانشانی وچ_گنتی;
	بنانشانی باہرلا_گنتی;
	بنية گرہ **باہرلا;
	حرف name[1];
};

بنية globals {
	بنية گرہ **nodes;
	بنانشانی nodes_len;
};
#تعریف_کرو G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() افعل { \
	تیاری_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.nodes = عدم; \
	G.nodes_len = 0; \
} طالما (0)

ساكن بنية گرہ *
get_گرہ(ثابت حرف *name)
{
	بنية گرہ *n;
	بنانشانی a = 0;
	بنانشانی b = G.nodes_len;

	/* Binary search for name */
	طالما (a != b) {
		بنانشانی m = (a + b) / 2;
		صحيح cmp = strcmp(name, G.nodes[m]->name);
		إذا (cmp == 0)
			أرجع G.nodes[m]; /*لبھیا*/
		إذا (cmp < 0) {
			b = m;
		} وإلا {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(حجم(*n) + لڑی_لمبائی(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * حجم(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * حجم(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	أرجع n;
}

ساكن فراغ
جوڑو_کنارا(بنية گرہ *a, بنية گرہ *b)
{
	a->باہرلا = xrealloc_سمتیہ(a->باہرلا, 6, a->باہرلا_گنتی);
	a->باہرلا[a->باہرلا_گنتی++] = b;
	b->وچ_گنتی++;
}

صحيح tsort_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح tsort_main(صحيح argc بن_ورتیا_PARAM, حرف **argv)
{
	حرف *سطر;
	ناپ_t linesz;
	ssize_t len;
	بنية گرہ *a;
	صحيح چکراں;
	بنانشانی i;
#إذا ENABLE_FEATURE_CLEAN_اتے
	بنانشانی max_len;
#جے_انت

	INIT_G();

	إذا (argv[1]) {
		إذا (argv[2])
			bb_show_usage();
		إذا (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = عدم;
	سطر = عدم;
	linesz = 0;
	طالما ((len = getline(&سطر, &linesz, stdin)) != -1) {
		حرف *s = سطر;
		طالما (*(s = skip_whitespace(s)) != '\0') {
			بنية گرہ *b;
			حرف *لفظ;

			لفظ = s;
			s = skip_non_whitespace(s);
			إذا (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_گرہ(لفظ);
			إذا (!a) {
				a = b;
			} وإلا {
				إذا (a != b)
					جوڑو_کنارا(a, b);
				a = عدم;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	إذا (a)
		bb_simple_error_پیغام_and_die("odd input");
	حرر(سطر);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	چکراں = 0;
#إذا ENABLE_FEATURE_CLEAN_اتے
	max_len = G.nodes_len;
#جے_انت
	طالما (G.nodes_len) {
		بنية گرہ *n;

		/* Search for first node with no incoming edges */
		لكل (i = 0; i < G.nodes_len; i++) {
			إذا (!G.nodes[i]->وچ_گنتی)
				اكسر;
		}
		إذا (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			چکراں++;
			i = 0;
#جے_بنا_تعریف TINY
			bb_error_پیغام("cycle at %s", G.nodes[i]->name);
#جے_انت
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#إذا ENABLE_FEATURE_CLEAN_اتے
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#جے_انت

		/* And remove its outgoing edges */
		لكل (i = 0; i < n->باہرلا_گنتی; i++)
			n->باہرلا[i]->وچ_گنتی--;

		puts(n->name);
	}
#إذا ENABLE_FEATURE_CLEAN_اتے
	لكل (i = 0; i < max_len; i++) {
		حرر(G.nodes[i]->باہرلا);
		حرر(G.nodes[i]);
	}
	حرر(G.nodes);
#جے_انت

	fflush_stdout_and_exit(چکراں ? 1 : 0);
}
