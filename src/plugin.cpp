#include <gcc-plugin.h>
#include <plugin-version.h>
#include <tree-pass.h>
#include <context.h>
#include <tree.h>
#include <basic-block.h>
#include <gimple.h>
#include <function.h>                                                            
#include <gimple-iterator.h>


void function_isol_print(function *fun)
{
	printf("\n\n\n");
	printf("/************************************************************************************************************************/\n");
	printf("/************************************************************************************************************************/\n");
	printf("/***********************************            %s               ***********************************/\n", function_name(fun) );
	printf("/************************************************************************************************************************/\n");
	printf("/************************************************************************************************************************/\n");
	printf("\n\n\n");


}

/**************** Generate graphviz **************/

/* Build a filename (as a string) based on function name */
	static char * 
cfgviz_generate_filename( function * fun, const char * suffix )
{
	char * target_filename ; 

	target_filename = (char *)xmalloc( 2048 * sizeof( char ) ) ;

	snprintf( target_filename, 1024, "%s_%s_%d_%s.dot", 
			current_function_name(),
			LOCATION_FILE( fun->function_start_locus ),
			LOCATION_LINE( fun->function_start_locus ),
			suffix ) ;

	return target_filename ;
}

/* Dump the graphviz representation of function 'fun' in file 'out' */
	static void 
cfgviz_internal_dump( function * fun, FILE * out ) 
{

	// Print the header line and open the main graph
	fprintf(out, "Digraph G{\n");


    basic_block bb;

    FOR_EACH_BB_FN (bb, cfun)
        {
            fprintf(out, "N%d [label=\"Basic Block %d\" shape=ellipse]\n", bb->index, bb->index);
            edge e;
            edge_iterator ei;

            FOR_EACH_EDGE(e, ei, bb->succs)
                {   
                    fprintf(out, "N%d -> N%d [color=red label=\"\"]\n", e->src->index, e->dest->index); 
                    if(e->flags & EDGE_FALLTHRU)
                        break;
                }   
        }


	// Close the main graph
	fprintf(out, "}\n");
}

	void 
cfgviz_dump( function * fun, const char * suffix )
{
	char * target_filename ; 
	FILE * out ;

	target_filename = cfgviz_generate_filename( fun, suffix ) ;

	printf( "[GRAPHVIZ] Generating CFG of function %s in file <%s>\n",
			current_function_name(), target_filename ) ;

	out = fopen( target_filename, "w" ) ;

	cfgviz_internal_dump( fun, out ) ;

	fclose( out ) ;
	free( target_filename ) ;
}


/**************** End - Generate graphviz **************/






int plugin_is_GPL_compatible;


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

/* Enum to represent the collective operations */
enum mpi_collective_code {
#define DEFMPICOLLECTIVES( CODE, NAME ) CODE,
#include "../include/MPI_collectives.def"
	LAST_AND_UNUSED_MPI_COLLECTIVE_CODE
#undef DEFMPICOLLECTIVES
} ;

/* Name of each MPI collective operations */
#define DEFMPICOLLECTIVES( CODE, NAME ) NAME,
const char *const mpi_collective_name[] = {
#include "../include/MPI_collectives.def"
} ;
#undef DEFMPICOLLECTIVES



class my_pass : public gimple_opt_pass {
    public:
        my_pass (gcc::context *ctxt)
            : gimple_opt_pass (my_pass_data, ctxt)
        {}
        my_pass *clone () { return new my_pass(g);}

        bool gate (function *fun) 
        {   
            function_isol_print(fun);
            printf("plugin: gate ... %s\n", function_name(fun)); 
            return true;
        }

        void detect_mpi_function(gimple* stmt)
        {
            if(is_gimple_call(stmt)){
 			    tree t ;
			    const char * callee_name ;

			    t = gimple_call_fndecl( stmt ) ;
			    callee_name = IDENTIFIER_POINTER(DECL_NAME(t));
                for(int i=0; i<5; i++)
                {
                    if(strcmp(callee_name, mpi_collective_name[i])==0){
                        printf("MPI function detected : %d\n", i);
                    }
                }
            }
        }

        void init_bb_bitmaps(function *fun)
		{
			basic_block bb;
			bitmap_head* bitmap_array = XNEWVEC(bitmap_head, last_basic_block_for_fn(cfun));
            FOR_ALL_BB_FN(bb, cfun)
            {
				bitmap_initialize(&bitmap_array[bb->index], &bitmap_default_obstack);
                bb->aux = &bitmap_array[bb->index];
			}
		}

        void free_bitmap(bitmap_head* bitmap)
        {
            bitmap_release(bitmap);
        }

		void free_bitmaps(function* fun){
			basic_block bb;
			FOR_EACH_BB_FN(bb, fun)
            {   
				bitmap_head *bmap = (bitmap_head*) bb->aux;
				free_bitmap(bmap);
			}
		}
        
        void nullify_aux(function *fun)
        {
            basic_block bb;
            FOR_EACH_BB_FN(bb, cfun)
            {
                bb->aux = nullptr;
            }

        }

        unsigned int execute (function *fun)
        {
            printf("plugin: execute my_pass on ... %s\n\n\n", current_function_name());

            basic_block bb;
	        gimple_stmt_iterator gsi;
	        gimple *stmt;
            FOR_EACH_BB_FN(bb, cfun)
            {
                for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
	            {
		            /* Get the current statement */
		            stmt = gsi_stmt (gsi);
                    detect_mpi_function(stmt);
                }
            }
            init_bb_bitmaps(cfun);
			free_bitmaps(cfun);
            nullify_aux(cfun);

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
