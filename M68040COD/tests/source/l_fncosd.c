/*   l_fncosd.c 1.2 1/21/92
*/

# this is a test for the library version of FPSP.A floating point
# exception bug was fixed in release2.2 of the lib version.this tests
# the software for no exception.this program should be compiled before testing.
#


main()
{
	double d,e,f;
  	
	d=0.0;
	e = facosd(0.0);
        printf("e is %f\n:",e);
	f = facosd(d);
	printf("f is %f",f);

}
