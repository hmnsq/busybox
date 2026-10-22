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

بنية ڳنڍ {
	بنانشان اندريون_ڳڻپ;
	بنانشان ٻاهريون_ڳڻپ;
	بنية ڳنڍ **ٻاهريون;
	حرف name[1];
};

بنية globals {
	بنية ڳنڍ **nodes;
	بنانشان nodes_len;
};
#وصف_ڪريو G (*(بنية globals*)bb_common_bufsiz1)
#وصف_ڪريو INIT_G() افعل { \
	تياري_common_bufsiz(); \
	BUILD_BUG_ON(حجم(G) > COMMON_BUFSIZE); \
	G.nodes = عدم; \
	G.nodes_len = 0; \
} طالما (0)

ساكن بنية ڳنڍ *
get_ڳنڍ(ثابت حرف *name)
{
	بنية ڳنڍ *n;
	بنانشان a = 0;
	بنانشان b = G.nodes_len;

	/* Binary search for name */
	طالما (a != b) {
		بنانشان m = (a + b) / 2;
		صحيح cmp = strcmp(name, G.nodes[m]->name);
		إذا (cmp == 0)
			أرجع G.nodes[m]; /*مليو*/
		إذا (cmp < 0) {
			b = m;
		} وإلا {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(حجم(*n) + لڙي_ڊيگهه(name));
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
جوڙيو_ڪنارو(بنية ڳنڍ *a, بنية ڳنڍ *b)
{
	a->ٻاهريون = xrealloc_ويڪٽر(a->ٻاهريون, 6, a->ٻاهريون_ڳڻپ);
	a->ٻاهريون[a->ٻاهريون_ڳڻپ++] = b;
	b->اندريون_ڳڻپ++;
}

صحيح tsort_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح tsort_main(صحيح argc اڻ_استعمال_ٿيل_PARAM, حرف **argv)
{
	حرف *سٽ;
	سائيز_t linesz;
	ssize_t len;
	بنية ڳنڍ *a;
	صحيح چڪرن;
	بنانشان i;
#إذا ENABLE_FEATURE_CLEAN_مٿي
	بنانشان max_len;
#جيڪڏهن_پورو

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
	سٽ = عدم;
	linesz = 0;
	طالما ((len = getline(&سٽ, &linesz, stdin)) != -1) {
		حرف *s = سٽ;
		طالما (*(s = skip_whitespace(s)) != '\0') {
			بنية ڳنڍ *b;
			حرف *لفظ_2;

			لفظ_2 = s;
			s = skip_non_whitespace(s);
			إذا (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_ڳنڍ(لفظ_2);
			إذا (!a) {
				a = b;
			} وإلا {
				إذا (a != b)
					جوڙيو_ڪنارو(a, b);
				a = عدم;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	إذا (a)
		bb_simple_error_نياپو_and_die("odd input");
	حرر(سٽ);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	چڪرن = 0;
#إذا ENABLE_FEATURE_CLEAN_مٿي
	max_len = G.nodes_len;
#جيڪڏهن_پورو
	طالما (G.nodes_len) {
		بنية ڳنڍ *n;

		/* Search for first node with no incoming edges */
		لكل (i = 0; i < G.nodes_len; i++) {
			إذا (!G.nodes[i]->اندريون_ڳڻپ)
				اكسر;
		}
		إذا (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			چڪرن++;
			i = 0;
#جيڪڏهن_وصف_نه_ٿيل TINY
			bb_error_نياپو("cycle at %s", G.nodes[i]->name);
#جيڪڏهن_پورو
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#إذا ENABLE_FEATURE_CLEAN_مٿي
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#جيڪڏهن_پورو

		/* And remove its outgoing edges */
		لكل (i = 0; i < n->ٻاهريون_ڳڻپ; i++)
			n->ٻاهريون[i]->اندريون_ڳڻپ--;

		puts(n->name);
	}
#إذا ENABLE_FEATURE_CLEAN_مٿي
	لكل (i = 0; i < max_len; i++) {
		حرر(G.nodes[i]->ٻاهريون);
		حرر(G.nodes[i]);
	}
	حرر(G.nodes);
#جيڪڏهن_پورو

	fflush_stdout_and_exit(چڪرن ? 1 : 0);
}
