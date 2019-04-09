

#include <check.h>

#include "../src/xrif.h"

START_TEST (handle_initialization)
{
   //Verify that all fields are initialized to their defaults.
   
   xrif_t hand;
   
   xrif_error_t rv = xrif_initialize_handle(&hand);
   
   ck_assert_int_eq( hand.width, 0);
   ck_assert_int_eq( hand.height, 0);
   ck_assert_int_eq( hand.depth, 0);
   ck_assert_int_eq( hand.frames, 0);
   ck_assert_int_eq( hand.type_code, 0);
   ck_assert_int_eq( hand.data_size, 0);
   ck_assert_int_eq( hand.compressed_size, 0);
   ck_assert_int_eq( hand.difference_method, XRIF_DIFFERENCE_DEFAULT);
   ck_assert_int_eq( hand.reorder_method, XRIF_REORDER_DEFAULT);
   ck_assert_int_eq( hand.compress_method, XRIF_COMPRESS_DEFAULT);
   ck_assert_int_eq( hand.lz4_acceleration, 1);
   ck_assert_int_eq( hand.omp_parallel, 0);
   ck_assert_int_eq( hand.omp_numthreads, 1);
   ck_assert_int_eq( hand.compress_on_raw, 1);
   ck_assert_int_eq( hand.own_raw, 0);
   ck_assert( hand.raw_buffer == NULL );
   ck_assert_int_eq( hand.raw_buffer_size, 0);
   ck_assert_int_eq( hand.own_reordered, 0);
   ck_assert( hand.reordered_buffer == NULL );
   ck_assert_int_eq( hand.reordered_buffer_size, 0);
   ck_assert_int_eq( hand.own_compressed, 0);
   ck_assert( hand.compressed_buffer == NULL );
   ck_assert_int_eq( hand.compressed_buffer_size, 0);
   
   ck_assert( rv == XRIF_NOERROR );
}
END_TEST

START_TEST (handle_setup)
{
   //Verfiy that the setup function sets only the expected members
   
   xrif_t hand;
   
   xrif_error_t rv = xrif_initialize_handle(&hand);
   
   ck_assert( rv == XRIF_NOERROR );
   
   rv = xrif_setup(&hand, 1024,64,32,1000, XRIF_TYPECODE_INT16);
   
   ck_assert_int_eq( hand.width, 1024);
   ck_assert_int_eq( hand.height, 64);
   ck_assert_int_eq( hand.depth, 32);
   ck_assert_int_eq( hand.frames, 1000);
   ck_assert_int_eq( hand.type_code, XRIF_TYPECODE_INT16);
   ck_assert_int_eq( hand.data_size, sizeof(int16_t));
   
   //And we check that everything else is unaltered
   ck_assert_int_eq( hand.compressed_size, 0);
   ck_assert_int_eq( hand.difference_method, XRIF_DIFFERENCE_DEFAULT);
   ck_assert_int_eq( hand.reorder_method, XRIF_REORDER_DEFAULT);
   ck_assert_int_eq( hand.compress_method, XRIF_COMPRESS_DEFAULT);
   ck_assert_int_eq( hand.lz4_acceleration, 1);
   ck_assert_int_eq( hand.omp_parallel, 0);
   ck_assert_int_eq( hand.omp_numthreads, 1);
   ck_assert_int_eq( hand.compress_on_raw, 1);
   ck_assert_int_eq( hand.own_raw, 0);
   ck_assert( hand.raw_buffer == NULL );
   ck_assert_int_eq( hand.raw_buffer_size, 0);
   ck_assert_int_eq( hand.own_reordered, 0);
   ck_assert( hand.reordered_buffer == NULL );
   ck_assert_int_eq( hand.reordered_buffer_size, 0);
   ck_assert_int_eq( hand.own_compressed, 0);
   ck_assert( hand.compressed_buffer == NULL );
   ck_assert_int_eq( hand.compressed_buffer_size, 0);
   
   ck_assert( rv == XRIF_NOERROR );
}
END_TEST

START_TEST (header_write)
{
   //This test verifies that header fields are correctly populated
   xrif_t hand;
   
   xrif_error_t rv = xrif_initialize_handle(&hand);
   
   ck_assert( rv == XRIF_NOERROR );
   
   rv = xrif_setup(&hand, 120,120,1,1000, XRIF_TYPECODE_INT16);
      
   ck_assert( rv == XRIF_NOERROR );
   
   
   char header[XRIF_HEADER_SIZE];
   
   rv = xrif_write_header( header, &hand );

   ck_assert( rv == XRIF_NOERROR );
   
   ck_assert( header[0] == 'x' );
   ck_assert( header[1] == 'r' );
   ck_assert( header[2] == 'i' );
   ck_assert( header[3] == 'f' );
 
   ck_assert( *((uint32_t *) &header[4]) == XRIF_VERSION);   
   ck_assert( *((uint32_t *) &header[8]) == XRIF_HEADER_SIZE); 
   ck_assert( *((uint32_t *) &header[12]) == hand.width);
   ck_assert( *((uint32_t *) &header[16]) == hand.height);
   ck_assert( *((uint32_t *) &header[20]) == hand.depth);
   ck_assert( *((uint32_t *) &header[24]) == hand.frames);
   ck_assert( *((uint16_t *) &header[28]) == hand.type_code);
   ck_assert( *((uint16_t *) &header[30]) == hand.difference_method);
   ck_assert( *((uint16_t *) &header[32]) == hand.reorder_method);
   ck_assert( *((uint16_t *) &header[34]) == hand.compress_method);
   ck_assert( *((uint16_t *) &header[36]) == hand.lz4_acceleration);
   ck_assert( *((uint16_t *) &header[38]) == 0);
   ck_assert( *((uint64_t *) &header[40]) == 0);
}
END_TEST

START_TEST (header_read)
{
   //This test writes a setup handle to a header
   //Then reads it to a new handle, verifying that members are set properly.
   
   xrif_t hand;
   
   xrif_error_t rv = xrif_initialize_handle(&hand);
   
   ck_assert( rv == XRIF_NOERROR );
   
   rv = xrif_setup(&hand, 120,240,2,1000, XRIF_TYPECODE_INT16);
      
   ck_assert( rv == XRIF_NOERROR );
   
   hand.lz4_acceleration = 10; //Change this from init value
   
   char header[XRIF_HEADER_SIZE];
   
   rv = xrif_write_header( header, &hand );

   ck_assert( rv == XRIF_NOERROR );
   
   //New handle to populate with read from header
   xrif_t hand2;
   
   rv = xrif_initialize_handle(&hand2);
   
   ck_assert( rv == XRIF_NOERROR );
   
   uint32_t header_size;
   rv = xrif_read_header( &hand2, &header_size, header );

   ck_assert( rv == XRIF_NOERROR );

   ck_assert_int_eq( header_size, XRIF_HEADER_SIZE);
   ck_assert_int_eq( hand2.width, 120);
   ck_assert_int_eq( hand2.height, 240);
   ck_assert_int_eq( hand2.depth, 2);
   ck_assert_int_eq( hand2.frames, 1000);
   ck_assert_int_eq( hand2.type_code, XRIF_TYPECODE_INT16);
   ck_assert_int_eq( hand2.data_size, sizeof(int16_t));
   ck_assert_int_eq( hand2.lz4_acceleration, 10);
   
   //And we check that everything else is unaltered
   ck_assert_int_eq( hand2.compressed_size, 0);
   ck_assert_int_eq( hand2.difference_method, XRIF_DIFFERENCE_DEFAULT);
   ck_assert_int_eq( hand2.reorder_method, XRIF_REORDER_DEFAULT);
   ck_assert_int_eq( hand2.compress_method, XRIF_COMPRESS_DEFAULT);
   ck_assert_int_eq( hand2.omp_parallel, 0);
   ck_assert_int_eq( hand2.omp_numthreads, 1);
   ck_assert_int_eq( hand2.compress_on_raw, 1);
   ck_assert_int_eq( hand2.own_raw, 0);
   ck_assert( hand2.raw_buffer == NULL );
   ck_assert_int_eq( hand2.raw_buffer_size, 0);
   ck_assert_int_eq( hand2.own_reordered, 0);
   ck_assert( hand2.reordered_buffer == NULL );
   ck_assert_int_eq( hand2.reordered_buffer_size, 0);
   ck_assert_int_eq( hand2.own_compressed, 0);
   ck_assert( hand2.compressed_buffer == NULL );
   ck_assert_int_eq( hand2.compressed_buffer_size, 0);

}

END_TEST

Suite * initandalloc_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Initialize and Allocate");

    /* Core test case */
    tc_core = tcase_create("Initial Setups");

    tcase_add_test(tc_core, handle_initialization);
    tcase_add_test(tc_core, handle_setup );
    suite_add_tcase(s, tc_core);

    return s;
}

Suite * headerformat_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Header Formatting");

    /* Core test case */
    tc_core = tcase_create("Write and Read No Errors");

    tcase_add_test(tc_core, header_write );
    tcase_add_test(tc_core, header_read );
    suite_add_tcase(s, tc_core);

    return s;
}

int main()
{
   int number_failed;
   Suite *s;
   SRunner *sr;

   s = initandalloc_suite();
   sr = srunner_create(s);

   srunner_run_all(sr, CK_NORMAL);
   number_failed = srunner_ntests_failed(sr);
   srunner_free(sr);
   
   s = headerformat_suite();
   sr = srunner_create(s);

   srunner_run_all(sr, CK_NORMAL);
   number_failed = srunner_ntests_failed(sr);
   srunner_free(sr);
      
   return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
   
}
