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

#شامل "libbb.h"
#شامل "common_bufsiz.h"

ساخت گرہ {
	غیرعلامتی اندر_گنتی;
	غیرعلامتی برآمد_گنتی;
	ساخت گرہ **برآمد;
	حرف name[1];
};

ساخت globals {
	ساخت گرہ **nodes;
	غیرعلامتی nodes_len;
};
#تعریف_کرو G (*(ساخت globals*)bb_common_bufsiz1)
#تعریف_کرو INIT_G() کرو { \
	تیاری_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.nodes = تہی; \
	G.nodes_len = 0; \
} جب (0)

ساکن ساخت گرہ *
get_گرہ(ثابت حرف *name)
{
	ساخت گرہ *n;
	غیرعلامتی a = 0;
	غیرعلامتی b = G.nodes_len;

	/* Binary search for name */
	جب (a != b) {
		غیرعلامتی m = (a + b) / 2;
		صحیح cmp = strcmp(name, G.nodes[m]->name);
		اگر (cmp == 0)
			واپس G.nodes[m]; /*ملا*/
		اگر (cmp < 0) {
			b = m;
		} ورنہ {
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
	واپس n;
}

ساکن خالی
جوڑو_کنارہ(ساخت گرہ *a, ساخت گرہ *b)
{
	a->برآمد = xrealloc_سمتیہ(a->برآمد, 6, a->برآمد_گنتی);
	a->برآمد[a->برآمد_گنتی++] = b;
	b->اندر_گنتی++;
}

صحیح tsort_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح tsort_main(صحیح argc غیر_مستعمل_PARAM, حرف **argv)
{
	حرف *سطر;
	سائز_t linesz;
	ssize_t len;
	ساخت گرہ *a;
	صحیح ادوار;
	غیرعلامتی i;
#اگر ENABLE_FEATURE_CLEAN_اوپر
	غیرعلامتی max_len;
#اگر_ختم

	INIT_G();

	اگر (argv[1]) {
		اگر (argv[2])
			bb_show_usage();
		اگر (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = تہی;
	سطر = تہی;
	linesz = 0;
	جب ((len = getline(&سطر, &linesz, stdin)) != -1) {
		حرف *s = سطر;
		جب (*(s = skip_whitespace(s)) != '\0') {
			ساخت گرہ *b;
			حرف *لفظ;

			لفظ = s;
			s = skip_non_whitespace(s);
			اگر (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_گرہ(لفظ);
			اگر (!a) {
				a = b;
			} ورنہ {
				اگر (a != b)
					جوڑو_کنارہ(a, b);
				a = تہی;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	اگر (a)
		bb_simple_error_پیغام_and_die("odd input");
	آزاد(سطر);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	ادوار = 0;
#اگر ENABLE_FEATURE_CLEAN_اوپر
	max_len = G.nodes_len;
#اگر_ختم
	جب (G.nodes_len) {
		ساخت گرہ *n;

		/* Search for first node with no incoming edges */
		برائے (i = 0; i < G.nodes_len; i++) {
			اگر (!G.nodes[i]->اندر_گنتی)
				توڑو;
		}
		اگر (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			ادوار++;
			i = 0;
#اگر_غیر_تعریف_شدہ TINY
			bb_error_پیغام("cycle at %s", G.nodes[i]->name);
#اگر_ختم
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#اگر ENABLE_FEATURE_CLEAN_اوپر
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#اگر_ختم

		/* And remove its outgoing edges */
		برائے (i = 0; i < n->برآمد_گنتی; i++)
			n->برآمد[i]->اندر_گنتی--;

		puts(n->name);
	}
#اگر ENABLE_FEATURE_CLEAN_اوپر
	برائے (i = 0; i < max_len; i++) {
		آزاد(G.nodes[i]->برآمد);
		آزاد(G.nodes[i]);
	}
	آزاد(G.nodes);
#اگر_ختم

	fflush_stdout_and_exit(ادوار ? 1 : 0);
}
