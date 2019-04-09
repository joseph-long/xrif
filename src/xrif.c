/** \file xrif.c
  * \brief The eXtreme-ao Reordered Image Format: Definitions
  *
  * \author Jared R. Males (jaredmales@gmail.com)
  *
  * \ingroup xrif_files
  */

#include "xrif.h"

xrif_error_t xrif_initialize_handle( xrif_t * handle )
{
   handle->width = 0;
   handle->height = 0;
   handle->depth = 0;
   handle->frames = 0;
   
   handle->type_code = 0;
   
   handle->data_size = 0;

   handle->compressed_size = 0;
   
   handle->difference_method = XRIF_DIFFERENCE_DEFAULT;
   
   handle->reorder_method = XRIF_REORDER_DEFAULT;
   
   handle->compress_method = XRIF_COMPRESS_DEFAULT;
   
   handle->lz4_acceleration = 1;

   handle->omp_parallel = 0;
   handle->omp_numthreads = 1;
   
   handle->compress_on_raw = 1;
   
   handle->own_raw = 0;
   handle->raw_buffer = 0;
   handle->raw_buffer_size = 0;
   
   handle->own_reordered = 0;
   handle->reordered_buffer = 0;
   handle->reordered_buffer_size = 0;
   
   handle->own_compressed = 0;
   handle->compressed_buffer = 0;
   handle->compressed_buffer_size = 0;
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_setup( xrif_t * handle,
                         dimensionT w,
                         dimensionT h,
                         dimensionT d,
                         dimensionT f,
                         xrif_typecode_t c
                       )
{
   handle->width = w;
   handle->height = h;
   handle->depth = d;
   handle->frames = f;
   handle->type_code = c;
   
   handle->data_size = xrif_typesize(handle->type_code);
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_set_raw( xrif_t * handle,
                  char * raw,          
                  size_t size          
                )
{
   
   if(handle->raw_buffer && handle->own_raw )
   {
      free(handle->raw_buffer);
   }

   handle->own_raw = 0;

   handle->raw_buffer = raw;
   
   handle->raw_buffer_size = size;
   
   if((handle->raw_buffer != NULL && handle->raw_buffer_size == 0) || (handle->raw_buffer ==0 && handle->raw_buffer_size != 0)) 
   {
      return XRIF_ERROR_INVALID_SIZE;
   }
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_allocate_raw( xrif_t * handle )
{
   if(handle->raw_buffer && handle->own_raw )
   {
      free(handle->raw_buffer);
   }
   
   if(handle->width * handle->height * handle->depth * handle->frames * handle->data_size == 0) return XRIF_ERROR_NOT_SETUP;
   
   if(handle->compress_on_raw)
   {
      //Set size to be the larger of LZ4_compressBound and the data size:
   
      handle->raw_buffer_size = LZ4_compressBound(handle->width * handle->height * handle->depth * handle->frames * handle->data_size);
   
      if(handle->raw_buffer_size < handle->width * handle->height * handle->depth * handle->frames * handle->data_size) 
      {
         handle->raw_buffer_size= handle->width * handle->height * handle->depth * handle->frames * handle->data_size;
      }
   }
   else
   {
      //Just set to the minimum size
      handle->raw_buffer_size = handle->width * handle->height * handle->depth * handle->frames * handle->data_size;
   }
   
   handle->raw_buffer = (char *) malloc( handle->raw_buffer_size );
   
   if(handle->raw_buffer == NULL) 
   {
      handle->own_raw = 0;
      return XRIF_ERROR_MALLOC;
   }
   
   handle->own_raw = 1;
   
   return 0;
}

xrif_error_t xrif_set_reordered( xrif_t * handle,
                        char * reordered,
                        size_t size
                    )
{
   
   if(handle->reordered_buffer && handle->own_reordered )
   {
      free(handle->reordered_buffer);
   }
   
   handle->reordered_buffer = reordered;
   
   handle->reordered_buffer_size = size;
   
   handle->own_reordered = 0;
   
   
   if((handle->reordered_buffer != NULL && handle->reordered_buffer_size == 0) || (handle->reordered_buffer == 0 && handle->reordered_buffer_size != 0)) return XRIF_ERROR_INVALID_SIZE;
   
   return 0;
}

xrif_error_t xrif_allocate_reordered( xrif_t * handle )
{
   
   if(handle->reordered_buffer && handle->own_reordered)
   {
      free(handle->reordered_buffer);
   }
   
   handle->reordered_buffer = (char *) malloc( handle->width * handle->height * handle->depth * handle->frames * handle->data_size);
   
   if(handle->reordered_buffer == NULL) 
   {
      handle->own_reordered = 0;
      return XRIF_ERROR_MALLOC;
   }
   
   handle->own_reordered = 1;
   
   return 0;
}

xrif_error_t xrif_set_compressed( xrif_t * handle, 
                         char * compressed,    
                         size_t size           
                       )
{
   
   if(handle->compressed_buffer && handle->own_compressed )
   {
      free(handle->compressed_buffer);
   }

   handle->own_compressed = 0;

   handle->compressed_buffer = compressed;
   
   handle->compressed_buffer_size = size;
   
   if((handle->compressed_buffer != NULL && handle->compressed_buffer_size == 0) || (handle->compressed_buffer == 0 && handle->compressed_buffer_size != 0)) return XRIF_ERROR_INVALID_SIZE;
   
   
   if(handle->compressed_buffer_size < LZ4_compressBound(handle->width * handle->height * handle->depth * handle->frames * handle->data_size)) return XRIF_ERROR_INSUFFICIENT_SIZE;
   
   return 0;
}

xrif_error_t xrif_allocate_compressed( xrif_t * handle )
{
   if(handle->compressed_buffer && handle->own_compressed )
   {
      free(handle->compressed_buffer);
   }
   
   if(handle->width * handle->height * handle->depth * handle->frames * handle->data_size == 0) return XRIF_ERROR_NOT_SETUP;
   
   handle->compressed_buffer_size = LZ4_compressBound(handle->width * handle->height * handle->depth * handle->frames * handle->data_size);
   
   handle->compressed_buffer = (char *) malloc( handle->compressed_buffer_size );
   
   if(handle->compressed_buffer == NULL) 
   {
      handle->own_compressed = 0;
      return XRIF_ERROR_MALLOC;
   }
   
   handle->own_compressed = 1;
   
   return 0;
}


xrif_error_t xrif_allocate( xrif_t * handle,
                   dimensionT w,
                   dimensionT h,
                   dimensionT d,
                   dimensionT f,
                   xrif_typecode_t c
                 )
{
   xrif_error_t rv;
   rv = xrif_setup(handle, w, h, d, f, c);
   if(rv < 0) return rv;
   
   rv = xrif_allocate_raw(handle);
   if(rv < 0) return rv;
   
   rv = xrif_allocate_reordered(handle);
   if(rv < 0) return rv;
   
   if(handle->compress_on_raw == 0)
   {
      rv = xrif_allocate_compressed(handle);
      if(rv < 0) return rv;
   }
   
   return 0;

}

xrif_error_t xrif_destroy_handle( xrif_t * handle )
{
   if(handle->reordered_buffer && handle->own_reordered)
   {
      free(handle->reordered_buffer);
   }
   
   if(handle->raw_buffer && handle->own_raw )
   {
      free(handle->raw_buffer);
   }
   
   if(handle->compressed_buffer && handle->own_compressed)
   {
      free(handle->compressed_buffer);
   }
   
   xrif_initialize_handle(handle);
   
   return 0;
}

xrif_error_t xrif_write_header( char * header,
                                xrif_t * handle 
                              )
{
   header[0] = 'x';
   header[1] = 'r';
   header[2] = 'i';
   header[3] = 'f';
   
   *((uint32_t *) &header[4]) = XRIF_VERSION;
   
   *((uint32_t *) &header[8]) = XRIF_HEADER_SIZE; 

   *((uint32_t *) &header[12]) = handle->width;
   
   *((uint32_t *) &header[16]) = handle->height;
   
   *((uint32_t *) &header[20]) = handle->depth;
   
   *((uint32_t *) &header[24]) = handle->frames;
   
   *((uint16_t *) &header[28]) = handle->type_code;

   *((uint16_t *) &header[30]) = handle->difference_method;
   
   *((uint16_t *) &header[32]) = handle->reorder_method;

   *((uint16_t *) &header[34]) = handle->compress_method;

   memset(&header[36], 0, 12);
   
   if(handle->compress_method == XRIF_COMPRESS_LZ4)
   {
      *((uint16_t *) &header[36]) = handle->lz4_acceleration;
   }
   
   return XRIF_NOERROR;
   
}


xrif_error_t xrif_read_header( xrif_t * handle,
                               uint32_t * header_size,
                               char * header
                             )
{
   if( header[0] != 'x' || header[1] != 'r' || header[2] != 'i' || header[3] != 'f')
   {
      return XRIF_ERROR_BADHEADER;
   }
   
   if( *((uint32_t *) &header[4]) != XRIF_VERSION)
   {
      return XRIF_ERROR_WRONGVERSION;
   }
   
   *header_size = *((uint32_t *) &header[8]); 

   handle->width = *((uint32_t *) &header[12]);
   
   handle->height = *((uint32_t *) &header[16]);
   
   handle->depth = *((uint32_t *) &header[20]);
   
   handle->frames = *((uint32_t *) &header[24]);
   
   handle->type_code = *((uint16_t *) &header[28]);

   handle->data_size = xrif_typesize(handle->type_code);
   
   handle->difference_method = *((uint16_t *) &header[30]);
   
   handle->reorder_method = *((uint16_t *) &header[32]);

   handle->compress_method = *((uint16_t *) &header[34]);

   if(handle->compress_method == XRIF_COMPRESS_LZ4)
   {
      handle->lz4_acceleration = *((uint16_t *) &header[36]);
   }
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_encode( xrif_t * handle )
{
   xrif_error_t rv; 
   
   clock_gettime(CLOCK_REALTIME, &handle->ts_difference_start);
   
   rv = xrif_difference( handle);
   
   if( rv != XRIF_NOERROR ) return rv;
   
   clock_gettime(CLOCK_REALTIME, &handle->ts_reorder_start);
   
   rv = xrif_reorder(handle);
   
   if( rv != XRIF_NOERROR ) return rv;
   
   clock_gettime(CLOCK_REALTIME, &handle->ts_compress_start);

   rv = xrif_compress(handle);
   
   if( rv != XRIF_NOERROR ) return rv;
   
   clock_gettime(CLOCK_REALTIME, &handle->ts_compress_done);

   return XRIF_NOERROR;
}

xrif_error_t xrif_decode( xrif_t * handle )
{
   xrif_error_t rv;
   
   clock_gettime(CLOCK_REALTIME, &handle->ts_decompress_start);
   
   rv = xrif_decompress(handle);
   
   if( rv != XRIF_NOERROR ) return rv;
    
   clock_gettime(CLOCK_REALTIME, &handle->ts_unreorder_start);
   
   rv = xrif_unreorder(handle);
   
   if( rv != XRIF_NOERROR ) return rv;
    
   clock_gettime(CLOCK_REALTIME, &handle->ts_undifference_start);
      
   rv = xrif_undifference(handle);

   if( rv != XRIF_NOERROR ) return rv;
    
   clock_gettime(CLOCK_REALTIME, &handle->ts_undifference_done);
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_difference( xrif_t * handle )
{
   int method = handle->difference_method;
   
   if(method == 0) method = XRIF_DIFFERENCE_DEFAULT;
   
   switch( method )
   {
      case XRIF_DIFFERENCE_PREVIOUS:
         return xrif_difference_previous_sint16(handle);
      case XRIF_DIFFERENCE_FIRST:
         return xrif_difference_first_sint16(handle);
      default:
         return XRIF_ERROR_NOTIMPL;
   }
}

xrif_error_t xrif_undifference( xrif_t * handle )
{
   int method = handle->difference_method;
   
   if(method == 0) method = XRIF_DIFFERENCE_DEFAULT;
   
   switch( method )
   {
      case XRIF_DIFFERENCE_PREVIOUS:
         return xrif_undifference_previous_sint16(handle);
      case XRIF_DIFFERENCE_FIRST:
         return XRIF_ERROR_NOTIMPL;
      default:
         return XRIF_ERROR_NOTIMPL;
   }
}

xrif_error_t xrif_difference_previous_sint16( xrif_t * handle )
{
   for(int n=0; n < handle->frames-1; ++n)
   {
      size_t n_stride0 = (handle->frames - 2 - n) * handle->height*handle->width*handle->depth;
      size_t n_stride =  (handle->frames - 1 - n) * handle->height*handle->width*handle->depth;

      for(int kk=0; kk< handle->depth; ++kk)
      {
         size_t kk_stride = kk*handle->height*handle->width;
         
         for(int ii=0; ii< handle->width; ++ii)
         {
            size_t ii_stride = ii*handle->height;
         
            for(int jj=0; jj< handle->height; ++jj)
            {
               size_t idx0 =  n_stride0 + kk_stride + ii_stride  + jj;
               size_t idx = n_stride + kk_stride + ii_stride  + jj;
            
               ((short *) handle->raw_buffer)[idx] = (((short *)handle->raw_buffer)[idx] - ((short*)handle->raw_buffer)[idx0]);
            }
         }
         
      }
   } 
   
   
   return 0;
}

xrif_error_t xrif_difference_first_sint16( xrif_t * handle )
{
   for(int n=0; n < handle->frames-1; ++n)
   {
      size_t n_stride0 = 0;// * handle->height*handle->width*handle->depth;
      size_t n_stride =  (handle->frames - 1 - n) * handle->height*handle->width*handle->depth;

      for(int kk=0; kk< handle->depth; ++kk)
      {
         size_t kk_stride = kk*handle->height*handle->width;
         
         for(int ii=0; ii< handle->width; ++ii)
         {
            size_t ii_stride = ii*handle->height;
         
            for(int jj=0; jj< handle->height; ++jj)
            {
               size_t idx0 =  n_stride0 + kk_stride + ii_stride  + jj;
               size_t idx = n_stride + kk_stride + ii_stride  + jj;
            
               ((short *) handle->raw_buffer)[idx] = ((short *)handle->raw_buffer)[idx] - ((short*)handle->raw_buffer)[idx0];
            }
         }
      }
   }   
   return 0;
}



xrif_error_t xrif_difference_sint16_rgb( xrif_t * handle )
{
   for(int n=0; n < handle->frames-1; ++n)
   {
      if( (handle->frames - 1 - n) % 3 == 0)
      {
         size_t n_stride0 = (handle->frames - 4 - n) * handle->height*handle->width;
         size_t n_stride =  (handle->frames - 1 - n) * handle->height*handle->width;
      
         for(int ii=0; ii< handle->width; ++ii)
         {
            size_t ii_stride = ii*handle->height;
        
            for(int jj=0; jj< handle->height; ++jj)
            {
               size_t idx0 =  n_stride0  + ii_stride  + jj;
               size_t idx = n_stride + ii_stride  + jj;
            
               ((short *) handle->raw_buffer)[idx] = (((short *)handle->raw_buffer)[idx] - ((short*)handle->raw_buffer)[idx0]);
            }
         }
      }
      else
      {
         size_t n_stride0 = (handle->frames - 2 - n) * handle->height*handle->width;
         size_t n_stride =  (handle->frames - 1 - n) * handle->height*handle->width;
      
         
         for(int ii=0; ii< handle->width; ++ii)
         {
            size_t ii_stride = ii*handle->height;
        
            for(int jj=0; jj< handle->height; ++jj)
            {
               size_t idx0 =  n_stride0  + ii_stride  + jj;
               size_t idx = n_stride + ii_stride  + jj;
            
               ((short *) handle->raw_buffer)[idx] = (((short *)handle->raw_buffer)[idx] - ((short*)handle->raw_buffer)[idx0]);
            }
         }
      }
   }
   
   for(int n=0; n < handle->frames-3; ++n)
   {
      if( (handle->frames - 1 - n) % 3 == 0) continue;
      else
      {
         size_t n_stride0 = (handle->frames - 4 - n) * handle->height*handle->width;
         size_t n_stride =  (handle->frames - 1 - n) * handle->height*handle->width;
      
         
         for(int ii=0; ii< handle->width; ++ii)
         {
            size_t ii_stride = ii*handle->height;
        
            for(int jj=0; jj< handle->height; ++jj)
            {
               size_t idx0 =  n_stride0  + ii_stride  + jj;
               size_t idx = n_stride + ii_stride  + jj;
            
               ((short *) handle->raw_buffer)[idx] = (((short *)handle->raw_buffer)[idx] - ((short*)handle->raw_buffer)[idx0]);
            }
         }
      }
   }
   return 0;
}

xrif_error_t xrif_undifference_previous_sint16( xrif_t * handle )
{
   for(int n=1; n < handle->frames; ++n)
   {
      size_t n_stride0 = (n-1) * handle->height*handle->width*handle->depth;
      size_t n_stride =  n * handle->height*handle->width*handle->depth;
      
      for(int kk=0; kk<handle->depth; ++kk)
      {
         size_t kk_stride = kk*handle->height*handle->width;
         
      for(int ii=0; ii< handle->width; ++ii)
      {
         size_t ii_stride = ii*handle->height;
         
         for(int jj=0; jj< handle->height; ++jj)
         {
            size_t idx0 =  n_stride0 + kk_stride + ii_stride  + jj;
            size_t idx = n_stride + kk_stride + ii_stride  + jj;
            
            ((short *) handle->raw_buffer)[idx] = ((short *)handle->raw_buffer)[idx] + ((short*)handle->raw_buffer)[idx0];
         }
      }
      }
   }
   
   return 0;
}

xrif_error_t xrif_reorder( xrif_t * handle )
{
   int method = handle->reorder_method;
   
   if(method == 0) method = XRIF_REORDER_DEFAULT;
   
   switch( method )
   {
      case XRIF_REORDER_NONE:
         return xrif_reorder_none(handle);
      case XRIF_REORDER_BYTEPACK:
         return xrif_reorder_bytepack(handle);
      case XRIF_REORDER_BYTEPACK_RENIBBLE:
         return xrif_reorder_bytepack_renibble(handle);
      case XRIF_REORDER_BITPACK:
         return xrif_reorder_bitpack(handle);
      default:
         return XRIF_ERROR_NOTIMPL;
   }
}

xrif_error_t xrif_unreorder( xrif_t * handle )
{
   int method = handle->reorder_method;
   
   if(method == 0) method = XRIF_REORDER_DEFAULT;
   
   switch( method )
   {
      case XRIF_REORDER_NONE:
         return xrif_unreorder_none(handle);
      case XRIF_REORDER_BYTEPACK:
         return xrif_unreorder_bytepack(handle);
      case XRIF_REORDER_BYTEPACK_RENIBBLE:
         return xrif_unreorder_bytepack_renibble(handle);
      case XRIF_REORDER_BITPACK:
         return xrif_unreorder_bitpack(handle);
      default:
         return XRIF_ERROR_NOTIMPL;
   }
}


xrif_error_t xrif_reorder_none( xrif_t * handle )
{
   size_t npix = handle->width * handle->height * handle->depth * handle->frames; 
   
   memcpy(handle->reordered_buffer, handle->raw_buffer, npix*handle->data_size);
   
   return 0;
}

xrif_error_t xrif_reorder_bytepack( xrif_t * handle )
{
   
   size_t npix = handle->width * handle->height * handle->depth * (handle->frames-1); 
   
   char * raw_buffer = handle->raw_buffer + handle->width*handle->height* handle->depth *handle->data_size;
   char * reordered_buffer = handle->reordered_buffer + handle->width*handle->height* handle->depth *handle->data_size;
   char * reordered_buffer2 = reordered_buffer + npix;
   
   //#pragma omp parallel for
   for(size_t pix=0; pix<handle->width*handle->height* handle->depth*handle->data_size; ++pix)
   {
      handle->reordered_buffer[pix] = handle->raw_buffer[pix];
   }
   
   #ifndef XRIF_NO_OMP
   #pragma omp parallel if (handle->omp_parallel > 0) 
   {
   #pragma omp for
   #endif
   for(size_t pix = 0; pix < npix; ++pix)
   {
      //Note: a lookup table for this was found to be 2x slower than the following algorithm.
      int_fast8_t x2 = raw_buffer[2*pix];
      int_fast8_t x1 = raw_buffer[2*pix+1];

      if(x2 < 0)
      {
         if(x1 == -1)  x1 = 0;
         else if(x1 == 0) x1 = -1;
      }
         
       reordered_buffer[pix] = x2; 
       reordered_buffer2[pix] = x1;
      
   }
   
   #ifndef XRIF_NO_OMP
   }
   #endif

   return 0;
}

xrif_error_t xrif_reorder_bytepack_renibble( xrif_t * handle )
{
   #include "bitshift_and_nibbles.inc"
   
   size_t npix = handle->width * handle->height * handle->depth * (handle->frames-1); 
   
   short * raw_buffer = (short*)(handle->raw_buffer + handle->width*handle->height* handle->depth *handle->data_size);
   unsigned char * reordered_buffer = (unsigned char *) handle->reordered_buffer + handle->width*handle->height* handle->depth *handle->data_size;
   unsigned char * reordered_buffer2 = (unsigned char*) reordered_buffer + npix;
   
   memset(reordered_buffer2, 0, npix);
   
   for(size_t pix=0; pix<handle->width*handle->height* handle->depth *handle->data_size; ++pix)
   {
      handle->reordered_buffer[pix] = handle->raw_buffer[pix];
   }
   
   #ifndef XRIF_NO_OMP
   #pragma omp parallel if (handle->omp_parallel > 0) 
   {
   #pragma omp for
   #endif
   for(size_t pix = 0; pix < npix; ++pix)
   {
      /* This block of commented code is left in to document the algorithm implemented in the lookup table.
       * And maybe we'll implement some defines to avoid lookup tables . . .
       */
      /*
      int_fast16_t s = raw_buffer[pix];
      int_fast16_t sbit = (s < 0);

      s *= (1-2*sbit);
      
      unsigned short us = ( (s) << 1) | sbit;
       
      reordered_buffer[pix] = ((char *)(&us))[0]; //

      // Note: A pre-calculated table look-up for just nibble values produced slightly slower code.
      if(pix % 2 == 0)
      {
         reordered_buffer2[pix/2] += ((unsigned char *)(&us))[1] << 4;
         reordered_buffer2[pix/2 + npix/2] += ((unsigned char *)(&us))[1] & 240;
      }
      else
      {
         reordered_buffer2[pix/2] += (((unsigned char *)(&us))[1] & 15);
         reordered_buffer2[pix/2 + npix/2] += ((unsigned char *)(&us))[1] >> 4;
      }
      /**/

      const unsigned char * bsn = &bitshift_and_nibbles[ ((unsigned short) raw_buffer[pix]) * 6 + (pix&1)*3];
      reordered_buffer[pix] = (char) bsn[0];
      reordered_buffer2[pix/2] += bsn[1];
      reordered_buffer2[pix/2 + npix/2] += bsn[2];
      /**/
   }
   #ifndef XRIF_NO_OMP
   }
   #endif
   
   return 0;
}





xrif_error_t xrif_reorder_bitpack( xrif_t * handle )
{
   //#include "circle_left_shift_one.inc"
   #include "bit_to_position.inc"
   #include "set_bits.inc"
   
   for(size_t pix=0; pix<handle->width*handle->height* handle->depth *handle->data_size; ++pix)
   {
      handle->reordered_buffer[pix] = handle->raw_buffer[pix];
   }
   
   short * raw_buffer = (short *) (handle->raw_buffer + handle->width*handle->height* handle->depth *handle->data_size);
   unsigned short * reordered_buffer = (unsigned short *) (handle->reordered_buffer + handle->width* handle->depth *handle->height*handle->data_size);
   
   size_t npix = handle->width * handle->height * handle->depth * (handle->frames-1); 

   memset(reordered_buffer, 0, npix*2);
      
   size_t stride = npix/16;
   
   #ifndef XRIF_NO_OMP
   #pragma omp parallel if (handle->omp_parallel > 0) 
   {
   #pragma omp for
   #endif
  
   for(size_t pix = 0; pix<npix; ++pix)
   {
      size_t sbyte = pix/16;
     
      size_t sbyte8 = sbyte + 8*stride;
      //if(sbyte == 0) continue;
      int_fast8_t bit = pix % 16;
      
      
      int_fast16_t s = raw_buffer[pix];
      int_fast16_t sbit = (s < 0);

      s *= (1-2*sbit);
      
      unsigned short us = ( (s) << 1) | sbit;
      //Attempt with lookup table is slower, leaving this in to document this, possibly for future defines:
      //unsigned short us = left_shift_one[*((unsigned short *) &raw_buffer[pix])];

      size_t st0 = ((unsigned char *) &us)[0]*16*8 + bit*8;
      size_t st1 = ((unsigned char *) &us)[1]*16*8 + bit*8;
      
      for(int_fast8_t b = 0; b < 8; ++b)
      {  
         reordered_buffer[sbyte +  b*stride] += bit_to_position[st0 + b] ;
      }
      if(((unsigned char *) &us)[1] == 0) continue;
      
      //Get the number of set bits, and for this second byte we only process those bits.
      unsigned char nbits2 = set_bits[((unsigned char *) &us)[1]*9];
      const unsigned char* bits2 = &set_bits[((unsigned char *) &us)[1]*9 + 1];
      for(int_fast8_t b = 0; b < nbits2; ++b)
      {      
         reordered_buffer[sbyte8 +  bits2[b]*stride] +=  bit_to_position[st1 + bits2[b]];
      }
   }
   
   
   
   #ifndef XRIF_NO_OMP
   }
   #endif
   
   return 0;
}
      

         
xrif_error_t xrif_unreorder_none( xrif_t * handle )
{
   size_t npix = handle->width * handle->height * handle->depth * handle->frames; 
   
   memcpy(handle->raw_buffer, handle->reordered_buffer, npix*handle->data_size);
   
   return 0;
}

xrif_error_t xrif_unreorder_bytepack( xrif_t * handle )
{
   int_fast8_t x1, x2;

   char * raw_buffer = handle->raw_buffer + handle->width*handle->height* handle->depth *handle->data_size;
   char * reordered_buffer = handle->reordered_buffer + handle->width*handle->height* handle->depth *handle->data_size;
   size_t npix = handle->width * handle->height * handle->depth * (handle->frames-1); 
   
   for(size_t pix=0; pix<handle->width*handle->height* handle->depth *handle->data_size; ++pix)
   {
      handle->raw_buffer[pix] = handle->reordered_buffer[pix];
   }
   
   #ifndef XRIF_NO_OMP
   #pragma omp parallel if (handle->omp_parallel > 0) 
   {
   #pragma omp for
   #endif
   for(size_t pix = 0; pix < npix; ++pix)
   {
      x2 = reordered_buffer[pix]; 
      x1 = reordered_buffer[npix+pix];
         
      if(x2 < 0)
      {
         if(x1 == -1) x1 = 0;
         else if(x1 == 0) x1 = -1;
      }
         
      raw_buffer[2*pix] = x2;
      raw_buffer[2*pix+1] = x1;
         
   }
   
   #ifndef XRIF_NO_OMP
   }
   #endif
      
   return 0;
}

xrif_error_t xrif_unreorder_bytepack_renibble( xrif_t * handle )
{
   size_t npix = handle->width * handle->height * handle->depth * (handle->frames-1); 
   
   short * raw_buffer = (short*)(handle->raw_buffer + handle->width*handle->height* handle->depth *handle->data_size);
   unsigned char * reordered_buffer = (unsigned char *) handle->reordered_buffer + handle->width*handle->height* handle->depth *handle->data_size;
   unsigned char * reordered_buffer2 = (unsigned char*) reordered_buffer + npix;
   
   for(size_t pix=0; pix<handle->width*handle->height* handle->depth *handle->data_size; ++pix)
   {
      handle->raw_buffer[pix] = handle->reordered_buffer[pix];
   }
   
   #ifndef XRIF_NO_OMP
   #pragma omp parallel if (handle->omp_parallel > 0) 
   {
   #pragma omp for
   #endif
   for(size_t pix = 0; pix < npix; ++pix)
   {
      unsigned short byte1 = 0;
      byte1 = reordered_buffer[pix];
      
      unsigned short nib1 =0;
      unsigned short nib2 = 0;
      if(pix % 2 == 0)
      {
         nib1 |= (reordered_buffer2[pix/2]) >> 4; 
         nib2 |= reordered_buffer2[pix/2 + npix/2] &240; 
      }
      else
      {
         nib1 |= reordered_buffer2[pix/2] & 15;
         nib2 |= (reordered_buffer2[pix/2 + npix/2] & 15) << 4;
      }
      
      byte1 |= (nib1 << 8);
      byte1 |= (nib2 << 8);
      
      int_fast16_t sbit = (byte1 & 1);

      int_fast16_t s = (byte1 >> 1);
      
      raw_buffer[pix] = s*(1-2*sbit);

   }
   
   #ifndef XRIF_NO_OMP
   }
   #endif

   return 0;
}

xrif_error_t xrif_unreorder_bitpack( xrif_t * handle )
{

   for(size_t pix=0; pix<handle->width*handle->height* handle->depth *handle->data_size; ++pix)
   {
      handle->raw_buffer[pix] = handle->reordered_buffer[pix];
   }
   
   short * raw_buffer = (short *) (handle->raw_buffer + handle->width*handle->height* handle->depth *handle->data_size);
   unsigned short * reordered_buffer = (unsigned short *) (handle->reordered_buffer + handle->width*handle->height* handle->depth *handle->data_size);
   
   size_t npix = handle->width * handle->height * handle->depth * (handle->frames-1); 

   memset(raw_buffer, 0, npix*2);
   
   size_t stride = npix/16;
   
   #ifndef XRIF_NO_OMP
   #pragma omp parallel if (handle->omp_parallel > 0) 
   {
   #pragma omp for
   #endif
   for(size_t pix = 0; pix<npix; ++pix)
   {
      
      size_t sbyte = pix/16;
      size_t bit = pix % 16;

      char sbit = (reordered_buffer[sbyte] >> bit) & 1;
      
      for(int_fast8_t b=1; b<16;++b)
      {
         raw_buffer[pix] |= ((reordered_buffer[sbyte +  b*stride] >> bit) & 1) << (b-1);
      }
      if(sbit) raw_buffer[pix]*=-1;
      //raw_buffer[pix] *= (1-2*sbit);
      
   }
   
   #ifndef XRIF_NO_OMP
   }
   #endif

   return 0;
}



xrif_error_t xrif_compress( xrif_t * handle )
{
   int method = handle->compress_method;
   
   if(method == 0) method = XRIF_COMPRESS_DEFAULT;
   
   switch( method )
   {
      case XRIF_COMPRESS_NONE:
         return xrif_compress_none(handle);
      case XRIF_COMPRESS_LZ4:
         return xrif_compress_lz4(handle);
      default:
         return XRIF_ERROR_NOTIMPL;
   }
      
}

xrif_error_t xrif_decompress( xrif_t * handle )
{
   int method = handle->compress_method;
   
   if(method == 0) method = XRIF_COMPRESS_DEFAULT;
   
   switch( method )
   {
      case XRIF_COMPRESS_NONE:
         return xrif_decompress_none(handle);
      case XRIF_COMPRESS_LZ4:
         return xrif_decompress_lz4(handle);
      default:
         return XRIF_ERROR_NOTIMPL;
   }
      
}

xrif_error_t xrif_compress_none( xrif_t * handle )
{
   int size = handle->width*handle->height* handle->depth *handle->frames*handle->data_size;
   
   char *compressed_buffer;
   size_t compressed_size;
   
   if(handle->compress_on_raw) 
   {
      compressed_buffer = handle->raw_buffer;
      compressed_size = handle->raw_buffer_size;
   }
   else 
   {
      compressed_buffer = handle->compressed_buffer;
      compressed_size = handle->compressed_buffer_size;
   }
   
   if( compressed_size < size )
   {
      return XRIF_ERROR_INSUFFICIENT_SIZE;
   }
   
   handle->compressed_size = size;
   
   memcpy( compressed_buffer, handle->reordered_buffer, handle->compressed_size);
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_decompress_none( xrif_t * handle )
{
   int size = handle->width*handle->height*handle->frames* handle->depth *handle->data_size;
   
   char *compressed_buffer;
   size_t compressed_buffer_size;
   
   if(handle->compress_on_raw) 
   {
      compressed_buffer = handle->raw_buffer;
      compressed_buffer_size = handle->raw_buffer_size;
   }
   else 
   {
      compressed_buffer = handle->compressed_buffer;
      compressed_buffer_size = handle->compressed_buffer_size;
   }
   
   if(handle->reordered_buffer_size < compressed_buffer_size || size < compressed_buffer_size) 
   {
      return XRIF_ERROR_INSUFFICIENT_SIZE;
   }
   
   memcpy( handle->reordered_buffer, compressed_buffer, size);
   
   return XRIF_NOERROR;
}

xrif_error_t xrif_compress_lz4( xrif_t * handle )
{
   int size = handle->width*handle->height* handle->depth *handle->frames*handle->data_size;
   
   char *compressed_buffer;
   size_t compressed_size;
   
   if(handle->compress_on_raw) 
   {
      compressed_buffer = handle->raw_buffer;
      compressed_size = handle->raw_buffer_size;
   }
   else 
   {
      compressed_buffer = handle->compressed_buffer;
      compressed_size = handle->compressed_buffer_size;
   }
   
   handle->compressed_size = LZ4_compress_fast ( handle->reordered_buffer, compressed_buffer, size, compressed_size, handle->lz4_acceleration);
   
   
   return 0;
}

xrif_error_t xrif_decompress_lz4( xrif_t * handle )
{
   int size = handle->width*handle->height*handle->frames* handle->depth *handle->data_size;
   
   char *compressed_buffer;
   
   if(handle->compress_on_raw) 
   {
      compressed_buffer = handle->raw_buffer;
   }
   else 
   {
      compressed_buffer = handle->compressed_buffer;
   }
   
   int size_decomp = LZ4_decompress_safe (compressed_buffer, handle->reordered_buffer, handle->compressed_size, size);

   if(size != size_decomp) return -1;
   
   return 0;
}

size_t xrif_typesize( xrif_typecode_t type_code)
{
   switch( type_code )
   {
      case XRIF_TYPECODE_UINT8:
         return sizeof(uint8_t);
      case XRIF_TYPECODE_INT8:
         return sizeof(int8_t);
      case XRIF_TYPECODE_UINT16:
         return sizeof(uint16_t);
      case XRIF_TYPECODE_INT16:
         return sizeof(int16_t);
      case XRIF_TYPECODE_UINT32:
         return sizeof(uint32_t);
      case XRIF_TYPECODE_INT32:
         return sizeof(int32_t);
      case XRIF_TYPECODE_UINT64:
         return sizeof(uint64_t);
      case XRIF_TYPECODE_INT64:
         return sizeof(int64_t);
      case XRIF_TYPECODE_HALF:
         return sizeof(uint16_t);
      case XRIF_TYPECODE_FLOAT:
         return sizeof(float);
      case XRIF_TYPECODE_DOUBLE:
         return sizeof(double);
      case XRIF_TYPECODE_COMPLEX_FLOAT:
         return sizeof(float complex);
      case XRIF_TYPECODE_COMPLEX_DOUBLE:
         return sizeof(double complex);
   }
   
   return 0;
}