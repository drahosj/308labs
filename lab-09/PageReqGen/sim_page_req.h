


/**
 * I was thinking that the generator should create 26 tasks named A through Z.  This
 * should make the wavedrom managable and readable.  The tasks do not need to be sorted
 * in any order when they are converted to json, the simulator takes care of that.
 */

struct sim_page_req
{
	unsigned long  page;	   	// the memory page that is being requested
	unsigned long arrive_time; 	// the time the page request will arrive
};

void init_json_tasklist(void);
void add_sim_task_to_json(struct sim_page_req * task);
void export_json_tasklist(FILE* fp);
