/*
 * jpegint.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 1997-2009 by Guido Vollbeding.
 * It was modified by The libjpeg-turbo Project to include only code relevant
 * to libjpeg-turbo.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file provides common declarations for the various JPEG modules.
 * These declarations are considered internal to the JPEG library; most
 * applications using the library shouldn't need to include this file.
 */


/* Declarations for both compression & decompression */

typedef enum {            /* Operating modes for buffer controllers */
  JBUF_PASS_THRU,         /* Plain stripwise operation */
  /* Remaining modes require a full-image buffer to have been created */
  JBUF_SAVE_SOURCE,       /* Run source subobject only, save output */
  JBUF_CRANK_DEST,        /* Run dest subobject only, using saved data */
  JBUF_SAVE_AND_PASS      /* Run both subobjects, save output */
} J_BUF_MODE;

/* Values of global_state field (jdapi.c has some dependencies on ordering!) */
#define CSTATE_START    100     /* after create_compress */
#define CSTATE_SCANNING 101     /* start_compress done, write_scanlines OK */
#define CSTATE_RAW_OK   102     /* start_compress done, write_raw_data OK */
#define CSTATE_WRCOEFS  103     /* jpeg_write_coefficients done */
#define DSTATE_START    200     /* after create_decompress */
#define DSTATE_INHEADER 201     /* reading header markers, no SOS yet */
#define DSTATE_READY    202     /* found SOS, ready for start_decompress */
#define DSTATE_PRELOAD  203     /* reading multiscan file in start_decompress*/
#define DSTATE_PRESCAN  204     /* performing dummy pass for 2-pass quant */
#define DSTATE_SCANNING 205     /* start_decompress done, read_scanlines OK */
#define DSTATE_RAW_OK   206     /* start_decompress done, read_raw_data OK */
#define DSTATE_BUFIMAGE 207     /* expecting jpeg_start_output */
#define DSTATE_BUFPOST  208     /* looking for SOS/EOI in jpeg_finish_output */
#define DSTATE_RDCOEFS  209     /* reading file in jpeg_read_coefficients */
#define DSTATE_STOPPING 210     /* looking for EOI in jpeg_finish_decompress */


/* Declarations for compression modules */

/* Master control module */
struct jpeg_comp_master {
  void (*prepare_for_pass) (j_compress_ptr cinfo);
  void (*pass_startup) (j_compress_ptr cinfo);
  void (*finish_pass) (j_compress_ptr cinfo);

  /* State variables made visible to other modules */
  boolean call_pass_startup;    /* True if pass_startup must be called */
  boolean is_last_pass;         /* True during last pass */
};

/* Main buffer control (downsampled-data buffer) */
struct jpeg_c_main_controller {
  void (*start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*process_data) (j_compress_ptr cinfo, JSAMPARRAY input_buf,
                        JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail);
};

/* Compression preprocessing (downsampling input buffer control) */
struct jpeg_c_prep_controller {
  void (*start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*pre_process_data) (j_compress_ptr cinfo, JSAMPARRAY input_buf,
                            JDIMENSION *in_row_ctr, JDIMENSION in_rows_avail,
                            JSAMPIMAGE output_buf,
                            JDIMENSION *out_row_group_ctr,
                            JDIMENSION out_row_groups_avail);
};

/* Coefficient buffer control */
struct jpeg_c_coef_controller {
  void (*start_pass) (j_compress_ptr cinfo, J_BUF_MODE pass_mode);
  boolean (*compress_data) (j_compress_ptr cinfo, JSAMPIMAGE input_buf);
};

/* Colorspace conversion */
struct jpeg_color_converter {
  void (*start_pass) (j_compress_ptr cinfo);
  void (*color_convert) (j_compress_ptr cinfo, JSAMPARRAY input_buf,
                         JSAMPIMAGE output_buf, JDIMENSION output_row,
                         int num_rows);
};

/* Downsampling */
struct jpeg_downsampler {
  void (*start_pass) (j_compress_ptr cinfo);
  void (*downsample) (j_compress_ptr cinfo, JSAMPIMAGE input_buf,
                      JDIMENSION in_row_index, JSAMPIMAGE output_buf,
                      JDIMENSION out_row_group_index);

  boolean need_context_rows;    /* TRUE if need rows above & below */
};

/* Forward DCT (also controls coefficient quantization) */
struct jpeg_forward_dct {
  void (*start_pass) (j_compress_ptr cinfo);
  /* perhaps this should be an array??? */
  void (*forward_DCT) (j_compress_ptr cinfo, jpeg_component_info * compptr,
                       JSAMPARRAY sample_data, JBLOCKROW coef_blocks,
                       JDIMENSION start_row, JDIMENSION start_col,
                       JDIMENSION num_blocks);
};

/* Entropy encoding */
struct jpeg_entropy_encoder {
  void (*start_pass) (j_compress_ptr cinfo, boolean gather_statistics);
  boolean (*encode_mcu) (j_compress_ptr cinfo, JBLOCKROW *MCU_data);
  void (*finish_pass) (j_compress_ptr cinfo);
};

/* Marker writing */
struct jpeg_marker_writer {
  void (*write_file_header) (j_compress_ptr cinfo);
  void (*write_frame_header) (j_compress_ptr cinfo);
  void (*write_scan_header) (j_compress_ptr cinfo);
  void (*write_file_trailer) (j_compress_ptr cinfo);
  void (*write_tables_only) (j_compress_ptr cinfo);
  /* These routines are exported to allow insertion of extra markers */
  /* Probably only COM and APPn markers should be written this way */
  void (*write_marker_header) (j_compress_ptr cinfo, int marker,
                               unsigned int datalen);
  void (*write_marker_byte) (j_compress_ptr cinfo, int val);
};


/* Declarations for decompression modules */

/* Master control module */
struct jpeg_decomp_master {
  void (*prepare_for_output_pass) (j_decompress_ptr cinfo);
  void (*finish_output_pass) (j_decompress_ptr cinfo);

  /* State variables made visible to other modules */
  boolean is_dummy_pass;        /* True during 1st pass for 2-pass quant */
};


/* Input control module */
struct jpeg_input_controller {
  int (*consume_input) (j_decompress_ptr cinfo);
  void (*reset_input_controller) (j_decompress_ptr cinfo);
  void (*start_input_pass) (j_decompress_ptr cinfo);
  void (*finish_input_pass) (j_decompress_ptr cinfo);

  /* State variables made visible to other modules */
  boolean has_multiple_scans;   /* True if file has multiple scans */
  boolean eoi_reached;          /* True when EOI has been consumed */

#ifdef ANDROID
  int (*consume_input_build_huffman_index) (j_decompress_ptr cinfo, huffman_index *index, int scan_count);
  int (*consume_markers) (j_decompress_ptr cinfo, huffman_index *index, int scan_count);
#endif
};

/* Main buffer control (downsampled-data buffer) */
struct jpeg_d_main_controller {
  void (*start_pass) (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*process_data) (j_decompress_ptr cinfo, JSAMPARRAY output_buf,
                        JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);
};

/* Coefficient buffer control */
struct jpeg_d_coef_controller {
  void (*start_input_pass) (j_decompress_ptr cinfo);
  int (*consume_data) (j_decompress_ptr cinfo);
  void (*start_output_pass) (j_decompress_ptr cinfo);
  int (*decompress_data) (j_decompress_ptr cinfo, JSAMPIMAGE output_buf);
  /* Pointer to array of coefficient virtual arrays, or NULL if none */
  jvirt_barray_ptr *coef_arrays;

#ifdef ANDROID
  int  (*consume_data_build_huffman_index) (j_decompress_ptr cinfo,huffman_index* index, int scan_count);

 /* column number of the first and last tile, respectively */
 int column_left_boundary;
 int column_right_boundary;

 /* column number of the first and last MCU, respectively */
 int MCU_column_left_boundary;
 int MCU_column_right_boundary;

 /* the number of MCU columns to skip from the indexed MCU, iM,
  * to the requested MCU boundary, rM, where iM is the MCU that we sample
  * into our index and is the nearest one to the left of rM.
  */
 int MCU_columns_to_skip;

#endif
};

/* Decompression postprocessing (color quantization buffer control) */
struct jpeg_d_post_controller {
  void (*start_pass) (j_decompress_ptr cinfo, J_BUF_MODE pass_mode);
  void (*post_process_data) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                             JDIMENSION *in_row_group_ctr,
                             JDIMENSION in_row_groups_avail,
                             JSAMPARRAY output_buf, JDIMENSION *out_row_ctr,
                             JDIMENSION out_rows_avail);
};

/* Marker reading & parsing */
struct jpeg_marker_reader {
  void (*reset_marker_reader) (j_decompress_ptr cinfo);
  /* Read markers until SOS or EOI.
   * Returns same codes as are defined for jpeg_consume_input:
   * JPEG_SUSPENDED, JPEG_REACHED_SOS, or JPEG_REACHED_EOI.
   */
  int (*read_markers) (j_decompress_ptr cinfo);
  /* Read a restart marker --- exported for use by entropy decoder only */
  jpeg_marker_parser_method read_restart_marker;

  /* State of marker reader --- nominally internal, but applications
   * supplying COM or APPn handlers might like to know the state.
   */
  boolean saw_SOI;              /* found SOI? */
  boolean saw_SOF;              /* found SOF? */
  int next_restart_num;         /* next restart number expected (0-7) */
  unsigned int discarded_bytes; /* # of bytes skipped looking for a marker */

#ifdef ANDROID
  void (*get_sos_marker_position) (j_decompress_ptr cinfo,huffman_index *index);
  int current_sos_marker_position;
#endif
};

/* Entropy decoding */
struct jpeg_entropy_decoder {
  void (*start_pass) (j_decompress_ptr cinfo);
  boolean (*decode_mcu) (j_decompress_ptr cinfo, JBLOCKROW *MCU_data);

  /* This is here to share code between baseline and progressive decoders; */
  /* other modules probably should not use it */
  boolean insufficient_data;    /* set TRUE after emitting warning */

#ifdef ANDROID
  boolean (*decode_mcu_discard_coef) (j_decompress_ptr cinfo);
  void (*configure_huffman_decoder) (j_decompress_ptr cinfo,huffman_offset_data offset);
  void (*get_huffman_decoder_configuration) (j_decompress_ptr cinfo,huffman_offset_data *offset);
  huffman_index *index;
#endif
};

/* Inverse DCT (also performs dequantization) */
typedef void (*inverse_DCT_method_ptr) (j_decompress_ptr cinfo,
                                        jpeg_component_info * compptr,
                                        JCOEFPTR coef_block,
                                        JSAMPARRAY output_buf,
                                        JDIMENSION output_col);

struct jpeg_inverse_dct {
  void (*start_pass) (j_decompress_ptr cinfo);
  /* It is useful to allow each component to have a separate IDCT method. */
  inverse_DCT_method_ptr inverse_DCT[MAX_COMPONENTS];
};

/* Upsampling (note that upsampler must also call color converter) */
struct jpeg_upsampler {
  void (*start_pass) (j_decompress_ptr cinfo);
  void (*upsample) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                    JDIMENSION *in_row_group_ctr,
                    JDIMENSION in_row_groups_avail, JSAMPARRAY output_buf,
                    JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail);

  boolean need_context_rows;    /* TRUE if need rows above & below */
};

/* Colorspace conversion */
struct jpeg_color_deconverter {
  void (*start_pass) (j_decompress_ptr cinfo);
  void (*color_convert) (j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                         JDIMENSION input_row, JSAMPARRAY output_buf,
                         int num_rows);
};

/* Color quantization or color precision reduction */
struct jpeg_color_quantizer {
  void (*start_pass) (j_decompress_ptr cinfo, boolean is_pre_scan);
  void (*color_quantize) (j_decompress_ptr cinfo, JSAMPARRAY input_buf,
                          JSAMPARRAY output_buf, int num_rows);
  void (*finish_pass) (j_decompress_ptr cinfo);
  void (*new_color_map) (j_decompress_ptr cinfo);
};


/* Miscellaneous useful macros */

#undef MAX
#define MAX(a,b)        ((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)        ((a) < (b) ? (a) : (b))


/* We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an INT32 quantity.
 * It is only applied with constant shift counts.  SHIFT_TEMPS must be
 * included in the variables of any routine using RIGHT_SHIFT.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS     INT32 shift_temp;
#define RIGHT_SHIFT(x,shft)  \
        ((shift_temp = (x)) < 0 ? \
         (shift_temp >> (shft)) | ((~((INT32) 0)) << (32-(shft))) : \
         (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft)     ((x) >> (shft))
#endif


/* Compression module initialization routines */
EXTERN(void) jinit_compress_master (j_compress_ptr cinfo);
EXTERN(void) jinit_c_master_control (j_compress_ptr cinfo,
                                     boolean transcode_only);
EXTERN(void) jinit_c_main_controller (j_compress_ptr cinfo,
                                      boolean need_full_buffer);
EXTERN(void) jinit_c_prep_controller (j_compress_ptr cinfo,
                                      boolean need_full_buffer);
EXTERN(void) jinit_c_coef_controller (j_compress_ptr cinfo,
                                      boolean need_full_buffer);
EXTERN(void) jinit_color_converter (j_compress_ptr cinfo);
EXTERN(void) jinit_downsampler (j_compress_ptr cinfo);
EXTERN(void) jinit_forward_dct (j_compress_ptr cinfo);
EXTERN(void) jinit_huff_encoder (j_compress_ptr cinfo);
EXTERN(void) jinit_phuff_encoder (j_compress_ptr cinfo);
EXTERN(void) jinit_arith_encoder (j_compress_ptr cinfo);
EXTERN(void) jinit_marker_writer (j_compress_ptr cinfo);
/* Decompression module initialization routines */
EXTERN(void) jinit_master_decompress (j_decompress_ptr cinfo);
EXTERN(void) jinit_d_main_controller (j_decompress_ptr cinfo,
                                      boolean need_full_buffer);
EXTERN(void) jinit_d_coef_controller (j_decompress_ptr cinfo,
                                      boolean need_full_buffer);
EXTERN(void) jinit_d_post_controller (j_decompress_ptr cinfo,
                                      boolean need_full_buffer);
EXTERN(void) jinit_input_controller (j_decompress_ptr cinfo);
EXTERN(void) jinit_marker_reader (j_decompress_ptr cinfo);
EXTERN(void) jinit_huff_decoder (j_decompress_ptr cinfo);
EXTERN(void) jinit_phuff_decoder (j_decompress_ptr cinfo);
EXTERN(void) jinit_arith_decoder (j_decompress_ptr cinfo);
EXTERN(void) jinit_inverse_dct (j_decompress_ptr cinfo);
EXTERN(void) jinit_upsampler (j_decompress_ptr cinfo);
EXTERN(void) jinit_color_deconverter (j_decompress_ptr cinfo);
EXTERN(void) jinit_1pass_quantizer (j_decompress_ptr cinfo);
EXTERN(void) jinit_2pass_quantizer (j_decompress_ptr cinfo);
EXTERN(void) jinit_merged_upsampler (j_decompress_ptr cinfo);

#ifdef ANDROID
EXTERN(void) jinit_huff_decoder_no_data(j_decompress_ptr cinfo);
EXTERN(void) jpeg_decompress_per_scan_setup (j_decompress_ptr cinfo);
#endif

/* Memory manager initialization */
EXTERN(void) jinit_memory_mgr (j_common_ptr cinfo);

/* Utility routines in jutils.c */
EXTERN(long) jdiv_round_up (long a, long b);
EXTERN(long) jround_up (long a, long b);

#ifdef ANDROID
EXTERN(long) jmin (long a, long b);
#endif

EXTERN(void) jcopy_sample_rows (JSAMPARRAY input_array, int source_row,
                                JSAMPARRAY output_array, int dest_row,
                                int num_rows, JDIMENSION num_cols);
EXTERN(void) jcopy_block_row (JBLOCKROW input_row, JBLOCKROW output_row,
                              JDIMENSION num_blocks);
EXTERN(void) jzero_far (void * target, size_t bytestozero);

#ifdef ANDROID
EXTERN(void) jset_input_stream_position (j_decompress_ptr cinfo,
                    int offset);
EXTERN(void) jset_input_stream_position_bit (j_decompress_ptr cinfo,
                    int byte_offset, int bit_left, INT32 buf);

EXTERN(int) jget_input_stream_position (j_decompress_ptr cinfo);
#endif

/* Constant tables in jutils.c */
#if 0                           /* This table is not actually needed in v6a */
extern const int jpeg_zigzag_order[]; /* natural coef order to zigzag order */
#endif
extern const int jpeg_natural_order[]; /* zigzag coef order to natural order */

/* Arithmetic coding probability estimation tables in jaricom.c */
extern const INT32 jpeg_aritab[];

/* Suppress undefined-structure complaints if necessary. */

#ifdef INCOMPLETE_TYPES_BROKEN
#ifndef AM_MEMORY_MANAGER       /* only jmemmgr.c defines these */
struct jvirt_sarray_control { long dummy; };
struct jvirt_barray_control { long dummy; };
#endif
#endif /* INCOMPLETE_TYPES_BROKEN */
