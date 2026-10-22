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

بنية غوټه {
	بینښه دننه_شمېر;
	بینښه بهر_شمېر;
	بنية غوټه **بهر;
	حرف name[1];
};

بنية globals {
	بنية غوټه **nodes;
	بینښه nodes_len;
};
#تعریف_کړه G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کړه INIT_G() افعل { \
	تیاری_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.nodes = عدم; \
	G.nodes_len = 0; \
} طالما (0)

ساكن بنية غوټه *
get_غوټه(ثابت حرف *name)
{
	بنية غوټه *n;
	بینښه a = 0;
	بینښه b = G.nodes_len;

	/* Binary search for name */
	طالما (a != b) {
		بینښه m = (a + b) / 2;
		صحيح cmp = strcmp(name, G.nodes[m]->name);
		إذا (cmp == 0)
			أرجع G.nodes[m]; /*وموندل*/
		إذا (cmp < 0) {
			b = m;
		} وإلا {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(حجم(*n) + مزي_اوږدوالی(name));
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
جمع_کړه_څنډه(بنية غوټه *a, بنية غوټه *b)
{
	a->بهر = xrealloc_ویکټور(a->بهر, 6, a->بهر_شمېر);
	a->بهر[a->بهر_شمېر++] = b;
	b->دننه_شمېر++;
}

صحيح tsort_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح tsort_main(صحيح argc ناکارول_شوی_PARAM, حرف **argv)
{
	حرف *کرښه;
	اندازه_t linesz;
	ssize_t len;
	بنية غوټه *a;
	صحيح دورې;
	بینښه i;
#إذا ENABLE_FEATURE_CLEAN_پورته
	بینښه max_len;
#د_که_پای

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
	کرښه = عدم;
	linesz = 0;
	طالما ((len = getline(&کرښه, &linesz, stdin)) != -1) {
		حرف *s = کرښه;
		طالما (*(s = skip_whitespace(s)) != '\0') {
			بنية غوټه *b;
			حرف *کلمه;

			کلمه = s;
			s = skip_non_whitespace(s);
			إذا (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_غوټه(کلمه);
			إذا (!a) {
				a = b;
			} وإلا {
				إذا (a != b)
					جمع_کړه_څنډه(a, b);
				a = عدم;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	إذا (a)
		bb_simple_error_پیغام_and_die("odd input");
	حرر(کرښه);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	دورې = 0;
#إذا ENABLE_FEATURE_CLEAN_پورته
	max_len = G.nodes_len;
#د_که_پای
	طالما (G.nodes_len) {
		بنية غوټه *n;

		/* Search for first node with no incoming edges */
		لكل (i = 0; i < G.nodes_len; i++) {
			إذا (!G.nodes[i]->دننه_شمېر)
				اكسر;
		}
		إذا (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			دورې++;
			i = 0;
#که_نه_تعریف_شوی TINY
			bb_error_پیغام("cycle at %s", G.nodes[i]->name);
#د_که_پای
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#إذا ENABLE_FEATURE_CLEAN_پورته
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#د_که_پای

		/* And remove its outgoing edges */
		لكل (i = 0; i < n->بهر_شمېر; i++)
			n->بهر[i]->دننه_شمېر--;

		puts(n->name);
	}
#إذا ENABLE_FEATURE_CLEAN_پورته
	لكل (i = 0; i < max_len; i++) {
		حرر(G.nodes[i]->بهر);
		حرر(G.nodes[i]);
	}
	حرر(G.nodes);
#د_که_پای

	fflush_stdout_and_exit(دورې ? 1 : 0);
}
