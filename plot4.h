

extern uint64 *plot4_none_fn( char *source_data, uint64 *dest_argb ); // dummy function... no drawing..

extern uint64 *plot4_color0_scale1( char *source_data, uint64 *dest_data );
extern uint64 *plot4_color0_scale2( char *source_data, uint64 *dest_data );
extern uint64 *plot4_scale1( char *source_data, uint32 *dest_data );
extern uint64 *plot4_scale2( char *source_data , uint64 *dest_data );

extern unsigned char *dest_ptr_image;
extern unsigned int dest_bpr;

union dbPixel
{
	uint64 data;

	struct 
	{
		uint32 argb1;
		uint32 argb2;
	};
};

extern union dbPixel palette2[256];


