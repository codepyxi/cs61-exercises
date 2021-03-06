#include "linereader.h"

static void merge_lines(lineset* dst, lineset* srca, lineset* srcb);
static void split_lines(lineset* left, lineset* right, const lineset* src);


#define MERGE_THRESHOLD 1024

static void sort_lines(lineset* ls) {
    if (lineset_size(ls) <= MERGE_THRESHOLD)
        qsort(ls->lines, lineset_size(ls), sizeof(line), line_compare);
    else {
        lineset left, right;
        split_lines(&left, &right, ls);

        sort_lines(&left);
        sort_lines(&right);

        merge_lines(ls, &left, &right);
    }
}


static void swap_spare(lineset* ls, int need_data) {
    size_t sz = lineset_size(ls);
    line* tmp = ls->lines;
    ls->lines = ls->spare;
    ls->end_lines = ls->lines + sz;
    ls->spare = tmp;
    if (need_data)
        memcpy(ls->lines, tmp, sz * sizeof(line));
}

static void merge_lines(lineset* dst, lineset* srca, lineset* srcb) {
    if (srca->lines >= dst->lines && srca->lines < dst->end_lines)
        swap_spare(dst, 0);
    if (srcb->lines >= dst->lines && srcb->lines < dst->end_lines)
        swap_spare(srcb, 1);

    const line* a = srca->lines, *b = srcb->lines;
    line* d = dst->lines;

    while (a != srca->end_lines && b != srcb->end_lines) {
        if (line_compare(a, b) <= 0) {
            *d = *a;
            ++d, ++a;
        } else {
            *d = *b;
            ++d, ++b;
        }
    }

    memcpy(d, a, (srca->end_lines - a) * sizeof(line));
    memcpy(d, b, (srcb->end_lines - b) * sizeof(line));
}

static void split_lines(lineset* left, lineset* right, const lineset* src) {
    left->lines = src->lines;
    left->end_lines = &src->lines[lineset_size(src) / 2];
    left->spare = src->spare;
    right->lines = left->end_lines;
    right->end_lines = src->end_lines;
    right->spare = &src->spare[lineset_size(src) / 2];
}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        FILE* f = freopen(argv[1], "r", stdin);
        assert(f);
    }

    lineset* ls = read_lines(stdin);
    ls->spare = (line*) malloc(lineset_size(ls) * sizeof(line));
    sort_lines(ls);

    // print the results
    for (line* l = ls->lines; l != ls->end_lines; ++l) {
        size_t nw = fwrite(l->s, 1, l->len, stdout);
        assert(nw == l->len);
    }

    return 0;
}
