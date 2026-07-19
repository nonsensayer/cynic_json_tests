#define CYNIC_NO_PREFIXES
#define UNGRATEFUL_NO_PREFIXES
#include <cynic.h>

int main(int argc, char **argv) {
    u8 ch;
    Buffer buf;
    Json_Node *jn;

    init(MB(1));

    ASSERT(argc == 3);

    ch = argv[2][0];

    if (!file_load(STR(argv[1]), &buf, alloc_temp_get())) {
        UNREACHABLE();
    }

    jn = json_parse(string_from_buffer(buf), STR(argv[1]), alloc_temp_get());
    if (jn == NULL) return 10000;

    switch (ch) {
        case 'y':
            ASSERT(jn->kind != JSON_NODE_ERROR);
            break;
        case 'n':
            ASSERT(jn->kind == JSON_NODE_ERROR);
            break;
        case 'i':
            break;
    }
}
