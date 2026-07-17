#include <cynic.h>

int main(int argc, char **argv) {
    u8 ch;
    Un_Buffer buf;
    Cyn_Json_Node *jn;

    un_init(UN_MB(1));

    UN_ASSERT(argc == 3);

    ch = argv[2][0];

    if (!cyn_file_load(UN_STR(argv[1]), &buf, un_alloc_temp_get())) {
        UN_UNREACHABLE();
    }

    jn = cyn_json_parse(un_string_from_buffer(buf), UN_STR(argv[1]), un_alloc_temp_get());

    switch (ch) {
        case 'y':
            UN_ASSERT(jn != NULL);
            UN_ASSERT(jn->kind != CYN_JSON_NODE_ERROR);
            break;
        case 'n':
            if (jn != NULL) {
                UN_ASSERT(jn->kind == CYN_JSON_NODE_ERROR);
            }
            break;
        case 'i':
            break;
    }
}
