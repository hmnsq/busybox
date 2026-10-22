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

#అన్తర్భావ "libbb.h"
#అన్తర్భావ "common_bufsiz.h"

నిర్మాణం కణుపు {
	గుర్తులేని లోపల_లెక్క;
	గుర్తులేని బహిర్గతం_లెక్క;
	నిర్మాణం కణుపు **బహిర్గతం;
	అక్షరం name[1];
};

నిర్మాణం globals {
	నిర్మాణం కణుపు **nodes;
	గుర్తులేని nodes_len;
};
#నిర్వచించు G (*(నిర్మాణం globals*)bb_common_bufsiz1)
#నిర్వచించు INIT_G() చేయి { \
	అమరిక_common_bufsiz(); \
	BUILD_BUG_ON(పరిమాణం(G) > COMMON_BUFSIZE); \
	G.nodes = ఖాళీ; \
	G.nodes_len = 0; \
} అయితే (0)

స్థిర నిర్మాణం కణుపు *
get_కణుపు(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం కణుపు *n;
	గుర్తులేని a = 0;
	గుర్తులేని b = G.nodes_len;

	/* Binary search for name */
	అయితే (a != b) {
		గుర్తులేని m = (a + b) / 2;
		పూర్ణాంకం cmp = strcmp(name, G.nodes[m]->name);
		ఒకవేళ (cmp == 0)
			తిరిగి G.nodes[m]; /*దొరికింది*/
		ఒకవేళ (cmp < 0) {
			b = m;
		} లేకపోతే {
			a = m + 1;
		}
	}

	/* Allocate new node */
	n = xzalloc(పరిమాణం(*n) + అక్షరశ్రేణి_పొడవు(name));
	//n->in_count = 0;
	//n->out_count = 0;
	//n->out = NULL;
	strcpy(n->name, name);

	/* Insert to maintain sort */
	G.nodes = xrealloc(G.nodes, (G.nodes_len + 1) * పరిమాణం(*G.nodes));
	memmove(&G.nodes[a + 1], &G.nodes[a],
		(G.nodes_len - a) * పరిమాణం(*G.nodes));
	G.nodes[a] = n;
	G.nodes_len++;
	తిరిగి n;
}

స్థిర శూన్య
కూడు_అంచు(నిర్మాణం కణుపు *a, నిర్మాణం కణుపు *b)
{
	a->బహిర్గతం = xrealloc_సదిశ(a->బహిర్గతం, 6, a->బహిర్గతం_లెక్క);
	a->బహిర్గతం[a->బహిర్గతం_లెక్క++] = b;
	b->లోపల_లెక్క++;
}

పూర్ణాంకం tsort_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం tsort_main(పూర్ణాంకం argc వాడని_PARAM, అక్షరం **argv)
{
	అక్షరం *పంక్తి;
	కొలత_t linesz;
	ssize_t len;
	నిర్మాణం కణుపు *a;
	పూర్ణాంకం చక్రాలు;
	గుర్తులేని i;
#ఒకవేళ ENABLE_FEATURE_CLEAN_పైకి
	గుర్తులేని max_len;
#అయితే_ముగింపు

	INIT_G();

	ఒకవేళ (argv[1]) {
		ఒకవేళ (argv[2])
			bb_show_usage();
		ఒకవేళ (NOT_LONE_DASH(argv[1])) {
			close(STDIN_FILENO); /* == 0 */
			xopen(argv[1], O_RDONLY); /* fd will be 0 */
		}
	}

	/* Read in words separated by <blank>s */
	a = ఖాళీ;
	పంక్తి = ఖాళీ;
	linesz = 0;
	అయితే ((len = getline(&పంక్తి, &linesz, stdin)) != -1) {
		అక్షరం *s = పంక్తి;
		అయితే (*(s = skip_whitespace(s)) != '\0') {
			నిర్మాణం కణుపు *b;
			అక్షరం *మాట;

			మాట = s;
			s = skip_non_whitespace(s);
			ఒకవేళ (*s)
				*s++ = '\0';

			/* Create nodes and edges for each word pair */
			b = get_కణుపు(మాట);
			ఒకవేళ (!a) {
				a = b;
			} లేకపోతే {
				ఒకవేళ (a != b)
					కూడు_అంచు(a, b);
				a = ఖాళీ;
			}
		}
	}
// Most other tools do not check for input read error (treat them as EOF)
//	die_if_ferror(in, input_filename);
	ఒకవేళ (a)
		bb_simple_error_సందేశం_and_die("odd input");
	విడుదల(పంక్తి);

	/*
	 * Kahn's algorithm:
	 *   - find a node that has no incoming edges, print and remove it
	 *   - repeat until the graph is empty
	 *   - if any nodes are left, they form cycles.
	 */
	చక్రాలు = 0;
#ఒకవేళ ENABLE_FEATURE_CLEAN_పైకి
	max_len = G.nodes_len;
#అయితే_ముగింపు
	అయితే (G.nodes_len) {
		నిర్మాణం కణుపు *n;

		/* Search for first node with no incoming edges */
		కొరకు (i = 0; i < G.nodes_len; i++) {
			ఒకవేళ (!G.nodes[i]->లోపల_లెక్క)
				ఆపు;
		}
		ఒకవేళ (i == G.nodes_len) {
			/* Must be a cycle; arbitraily break it at node 0 */
			చక్రాలు++;
			i = 0;
#నిర్వచితం_కాకపోతే TINY
			bb_error_సందేశం("cycle at %s", G.nodes[i]->name);
#అయితే_ముగింపు
		}

		/* Remove the node (need no longer maintain sort) */
		n = G.nodes[i];
		G.nodes[i] = G.nodes[--G.nodes_len];
#ఒకవేళ ENABLE_FEATURE_CLEAN_పైకి
		/* Keep reference to removed node so it can be freed */
		G.nodes[G.nodes_len] = n;
#అయితే_ముగింపు

		/* And remove its outgoing edges */
		కొరకు (i = 0; i < n->బహిర్గతం_లెక్క; i++)
			n->బహిర్గతం[i]->లోపల_లెక్క--;

		puts(n->name);
	}
#ఒకవేళ ENABLE_FEATURE_CLEAN_పైకి
	కొరకు (i = 0; i < max_len; i++) {
		విడుదల(G.nodes[i]->బహిర్గతం);
		విడుదల(G.nodes[i]);
	}
	విడుదల(G.nodes);
#అయితే_ముగింపు

	fflush_stdout_and_exit(చక్రాలు ? 1 : 0);
}
