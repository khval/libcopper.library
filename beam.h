
struct ffdpart;

struct ffdpart
{
	uint32_t wc;
	uint32_t wcStart;
	uint32_t wcEnd;
	uint32_t flags;
	void (*fn) (struct ffdpart *this);
};

#define beam_bpr 128

enum
{
	f_skip = 0,
	f_window = 1,
	f_ddf = 2,
	f_display = 4
};

extern void *fns[];
extern struct ffdpart *bInfo;
extern struct ffdpart bInfos[128];
extern int beamParts;
extern uint32_t beam_x;
extern uint32_t beam_y;
extern uint32_t beam_remain;

extern int decodeBeam();
extern void sync_beam();
extern void printBeamInfo();

extern void beam_hidden();
extern void beam_displayed();
extern void beam_displayed_in_window();

extern const char *beam_fn_names[];

