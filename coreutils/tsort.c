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

بنية گره {
	بیعلامت درون_شمار;
	بیعلامت بیرون_شمار;
	بنية گره **بیرون;
	حرف name[1];
};

بنية globals {
	بنية گره **nodes;
	بیعلامت nodes_len;
};
#تعریف_کن G (*(بنية globals*)bb_common_bufsiz1)
#تعریف_کن INIT_G() افعل { \
	راهاندازی_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.nodes = عدم; \
	G.nodes_len = 0; \
} طالما (0)

ساكن بنية گره *
get_گره(ثابت حرف *name)
{
	بنية گره *n;
	بیعلامت a = 0;
	بیعلامت b = G.nodes_len;

	/* Binary search for name */
	طالما (a != b) {
		بیعلامت m = (a + b) / 2;
		صحيح cmp = strcmp(name, G.nodes[m]->name);
		إذا (cmp == 0)
			أرجع G.nodes[m]; /*یافت*/
		إذا (cmp < 0) {
			b = m;
		} وإلا {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(حجم(*n) + طول_رشته(name));
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
جمع_کن_یال(بنية گره *a, بنية گره *b)
{
	a->بیرون = xrealloc_وکتور(a->بیرون, 6, a->بیرون_شمار);
	a->بیرون[a->بیرون_شمار++] = b;
	b->درون_شمار++;
}

صحيح tsort_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح tsort_main(صحيح argc استفادهنشده_PARAM, حرف **argv)
{
	حرف *سطر;
	اندازه_t linesz;
	ssize_t len;
	بنية گره *a;
	صحيح چرخهها;
	بیعلامت i;
#إذا ENABLE_FEATURE_CLEAN_به_بالا
	بیعلامت max_len;
#پایان_اگر

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
			بنية گره *b;
			حرف *کلمه;

			کلمه = s;
			s = skip_non_whitespace(s);
			إذا (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_گره(کلمه);
			إذا (!a) {
				a = b;
			} وإلا {
				إذا (a != b)
					جمع_کن_یال(a, b);
				a = عدم;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	إذا (a)
		bb_simple_error_پیام_and_die("odd input");
	حرر(سطر);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	چرخهها = 0;
#إذا ENABLE_FEATURE_CLEAN_به_بالا
	max_len = G.nodes_len;
#پایان_اگر
	طالما (G.nodes_len) {
		بنية گره *n;

		/* Search for first node with no incoming edges */
		لكل (i = 0; i < G.nodes_len; i++) {
			إذا (!G.nodes[i]->درون_شمار)
				اكسر;
		}
		إذا (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			چرخهها++;
			i = 0;
#اگر_تعریف_نشده TINY
			bb_error_پیام("cycle at %s", G.nodes[i]->name);
#پایان_اگر
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#إذا ENABLE_FEATURE_CLEAN_به_بالا
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#پایان_اگر

		/* And remove its outgoing edges */
		لكل (i = 0; i < n->بیرون_شمار; i++)
			n->بیرون[i]->درون_شمار--;

		puts(n->name);
	}
#إذا ENABLE_FEATURE_CLEAN_به_بالا
	لكل (i = 0; i < max_len; i++) {
		حرر(G.nodes[i]->بیرون);
		حرر(G.nodes[i]);
	}
	حرر(G.nodes);
#پایان_اگر

	fflush_stdout_and_exit(چرخهها ? 1 : 0);
}
