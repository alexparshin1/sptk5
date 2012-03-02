The readme file contains the brief information
necessary to compile and run the example applications.

1) First, compile and install SPTK.

   On Unix, Linux, Solaris it will be

   cd /your_sptk_dir
   cmake .
   make
   make install

   The SPTK installs its shared libraries into /usr/local/lib. 
   This path should be a part of LD_LIBRARY_PATH, or you should move
   the libsptk*.so and libspdb*.so into /usr/lib.


   On Windows, you can simply use the MSVC++ project to build the the library (static) and examples.

2) Compile the examples.
   cd /your_sptk_dir/examples
   make

3) If you want the database-related examples to work, you will need 
   to configure the ODBC datasource in odbc.ini.

   If you'are using unixODBC on Unix, Linux, Solaris (www.unixodbc.org) 
   you can use the unixODBC GUI to configure the datasource, or simply edit
   the file /usr/local/ect/odbc.ini.

   On Windows, just use Control Panel -> ODBC Data Sources.

   The examples will try to create a temporary table(s) in your database. 
   That way any existing data in your database are safe, and it doesn't leave
   any traces - all the temporary tables are destroyed by the database server
   when application terminates the connection with the server.
   If your database doesn't support the temporary tables (MS Access, for instance)
   the 'CREATE TEMP TABLE' SQL statement can be replaced (in the example programs) 
   with just 'CREATE TABLE' statement, but in that case the example program will 
   attempt to create a regular table.
   In any case, if the table with such name already exists, the example program will 
   generate an error and exit without destroying the data.
