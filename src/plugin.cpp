#include <gcc-plugin.h>
#include <tree-pass.h>
#include <context.h>

int plugin_is_GPL_compatible;

/*
struct register_pass_info {
    opt_pass *pass;
    const char *reference_pass_name;
    int ref_pass_instance_number;
    enum pass_positioning_ops pos_op;
};

enum pass_positioning_ops {
    PASS_POS_INSERT_AFTER,
    PASS_POS_INSERT_BEFORE,
    PASS_POS_REPLACE
};
*/

const pass_data my_pass_data = {
    GIMPLE_PASS,
    "NEW_PASS",
    OPTGROUP_NONE,
    TV_OPTIMIZE,
    0,
    0,
    0,
    0,
    0,
};

class my_pass : public gimple_opt_pass {
    public:
        my_pass (gcc::context *ctxt)
            : gimple_opt_pass (my_pass_data, ctxt)
        {}
        my_pass *clone () { return new my_pass(g);}

        bool gate (function *fun) {printf("In function gate"); return true;}

        unsigned int execute (function *fun){
            printf("Executing my_pass with function %s\n", function_name(fun));
            return 0;
        }
};


int plugin_init(struct plugin_name_args * plugin_info, struct plugin_gcc_version * version){
	printf("plugin_init: Entering...\n");

    my_pass p(g);

    struct register_pass_info my_pass_info;

    my_pass_info.pass = &p;
    my_pass_info.reference_pass_name = "cfg";
    my_pass_info.ref_pass_instance_number = 0;
    my_pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback(
            plugin_info->base_name,
            PLUGIN_PASS_MANAGER_SETUP,
            NULL,
            &my_pass_info);

    return 0;
}
