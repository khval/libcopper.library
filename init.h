
struct kIcon
{
	struct Gadget *gadget ;
	struct Image *image ;
};

extern struct Library			*IntuitionBase;
extern struct IntuitionIFace	*IIntuition;

extern struct Library			*GraphicsBase;
extern struct GraphicsIFace	*IGraphics;

extern struct Library			*GadToolsBase;
extern struct GadToolsIFace	*IGadTools;

extern bool open_libs();
extern void close_libs();

extern BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface);
extern void close_lib_all( struct Library **Base, struct Interface **I );

extern void open_icon(struct Window *window,struct DrawInfo *dri, ULONG imageID, ULONG gadgetID, struct kIcon *icon );
extern void dispose_icon(struct Window *win, struct kIcon *icon);


#define close_lib(name) close_lib_all( &(name ## Base), (struct Interface **) &(I ## name) )

