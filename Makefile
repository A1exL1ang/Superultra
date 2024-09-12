all:
	
	clang++ -o superultra_20_64_ja glob.cpp -std=c++17 -Ofast \
	-funroll-loops -ftree-vectorize -finline-functions -pthread  -Wl,--stack,16777216      \
	-static -static-libgcc -static-libstdc++ -DNDEBUG -fuse-ld=lld -MMD -MP -fno-rtti -fstrict-aliasing \
	
	
	 
	

	
	#              for windows >       -Wl,--stack,16777216 

    #              for linux   >       -Wl,-z,stack-size=16777216



	#       -mpopcnt -msse4.1 -msse4.2 -mbmi -mfma -mavx2 -mbmi2 -mavx -march=native -mtune=native      <    avx/bmi2 enable
	
	
	#       -fprofile-instr-generate -fcoverage-mapping                                                 <   before -o
	
	#        llvm-profdata merge -output=default.profdata *.profraw                                     <  enter on command line
 
    #       -fprofile-use=default.profdata                                                              <   before -o
	
	
	

    #       -mpopcnt -mtune=znver2 -msse4.1 -mbmi -mfma -mavx2 -mavx -mbmi                              <  for Zen 2 processors (no bmi2)
    
    
    #       -march=silvermont -mtune=silvermont -msse4.1 -msse4.2 -mpopcnt                              <  for popcount builds (with sse4.1/4.2)    


    #       -msse3 -mssse3 -march=k8 -mtune=k8                                                          <  sse3 build	
   
