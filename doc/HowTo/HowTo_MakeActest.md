# How to create a CTest 

I will explain how to create a ctest for a plugin with the example of the Sunlight Plugin.

## How to create a simple ctest

+ In the folder where the files of your functionality to test are, create a new folder and name it "Test".

+ In this folder, create a .cpp file and name it `testMyFunctionality.cpp`.

+ Open this `testMyFunctionality.cpp` file and create a main which will hold the code of the test to launch:

		int main(int narg, char** argv)
		{
		    std::cout << "bah" << std::endl;
		    return 0;
		}

+ In the 'Test' folder, create a `CMakeLists.txt` file.

+ Open this `CMakeLists.txt` file and add the following lines :


		add_executable(testSunlightComputation testSunlightComputation.cxx)

		target_link_libraries(testSunlightComputation CityGMLSunlightQtPlugin)
		
		add_test( SUNLIGHT_COMPUTATION
		  testSunlightComputation)

*Note : the command `add_executable` permits to create the executable which will run when you launch the test. 
The first argument is the name of the executable which will be created and the second one is the .cpp files which will be used to create this executable.*

*Note : The command `target_link_libraries` permits to link against the libraries your test will need.
The first argument is the name of your previously created exec and the second one is the name of your library (normally set in the main CMakeLists.txt).*

*Note: The commande `add_test` is the command which will run when you launch your ctest.
The first argument is the name of the test and the second one is the name of the executable.*

+ Now go back in the folder holding your functionality and add the following line to the CMakeLists.txt : 

		add_subdirectory(Test)

+ Run CMake and build your project

+ Once your project is build, open a shell, go into your build folder and type the following commande line :

		ctest

You will see the following output :

	Test project /home/vincent/Documents/VCity_Project/build_debug
	    Start 1: SUNLIGHT_COMPUTATION
	1/1 Test #1: SUNLIGHT_COMPUTATION .............   Passed    0.00 sec
	
	100% tests passed, 0 tests failed out of 1
	
	Total Test time (real) =   0.00 sec


It means the test passed!

Now try the following command line :

		ctest -V

You will see the following output :

	UpdateCTestConfiguration  from :/home/vincent/Documents/VCity_Project/build_debug/DartConfiguration.tcl
	UpdateCTestConfiguration  from :/home/vincent/Documents/VCity_Project/build_debug/DartConfiguration.tcl
	Test project /home/vincent/Documents/VCity_Project/build_debug
	Constructing a list of tests
	Done constructing a list of tests
	Checking test dependency graph...
	Checking test dependency graph end
	test 1
	    Start 1: SUNLIGHT_COMPUTATION
	
	1: Test command: /home/vincent/Documents/VCity_Project/build_debug/src/plugins/CityGMLSunlightQtPlugin/Test/testSunlightComputation
	1: Test timeout computed to be: 9.99988e+06
	1: bah
	1/1 Test #1: SUNLIGHT_COMPUTATION .............   Passed    0.00 sec
	
	100% tests passed, 0 tests failed out of 1
	
	Total Test time (real) =   0.00 sec

As you can see, the -V option permits to output the std::cout, this can be useful for debugging when a test fails.


## How to add arguments to your test

You might want to add arguments to your test. I will show an example on how to do that :

+ Open the CMakeLists.txt file which is in "Test" folder and replace the line :

		add_test( SUNLIGHT_COMPUTATION
		  testSunlightComputation)

   By :

		add_test( SUNLIGHT_COMPUTATION
		  testSunlightComputation monkey)


*Note : monkey will be the argument you will pass to your test function. It can be an int value as well. You can add more argument after "monkey" by separatinf them with a space character. Ex: add_test( SUNLIGHT_COMPUTATION
  testSunlightComputation monkey 1).*

+ Open your `testMyFunctionality.cpp` file and add the following lines to the main function :

		std::string monkey_str(argv[1]);
		std::cout << monkey_str << std::endl;

+ Run cmake and build your project. 

+ Go back into your shell and run the `ctest -V` command. You should see the following output :

		UpdateCTestConfiguration  from :/home/vincent/Documents/VCity_Project/build_debug/DartConfiguration.tcl
		UpdateCTestConfiguration  from :/home/vincent/Documents/VCity_Project/build_debug/DartConfiguration.tcl
		Test project /home/vincent/Documents/VCity_Project/build_debug
		Constructing a list of tests
		Done constructing a list of tests
		Checking test dependency graph...
		Checking test dependency graph end
		test 1
		    Start 1: SUNLIGHT_COMPUTATION
		
		1: Test command: /home/vincent/Documents/VCity_Project/build_debug/src/plugins/CityGMLSunlightQtPlugin/Test/testSunlightComputation "monkey"
		1: Test timeout computed to be: 9.99988e+06
		1: monkey
		1/1 Test #1: SUNLIGHT_COMPUTATION .............   Passed    0.00 sec
		
		100% tests passed, 0 tests failed out of 1
		
		Total Test time (real) =   0.00 sec

*Note: Lets say you want to pass an int value as an argument to your test and your add_test command look like that : `add_test( SUNLIGHT_COMPUTATION
  testSunlightComputation monkey 1)`. In order to get an int value in the main, you must do the following : ` int iValue = std::stoi(argv[2]);`*

## How to add more test

If you want to add more test, for example with different arguments, you simply need to add another `add_test()` command in your CMakeLists.txt in Test folder and change the arguments.
