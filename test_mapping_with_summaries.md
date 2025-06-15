# Source to LLVM IR Mapping

<style>
table {
  width: 100%;
  table-layout: fixed;
  overflow-wrap: break-word;
}
th:first-child {
  width: 10%;
}
th:nth-child(2) {
  width: 20%;
}
th:nth-child(3) {
  width: 60%;
}
th:last-child {
  width: 10%;
}
</style>

| Source Line | Source Code | LLVM IR | Summary |
| ----------: | ----------- | ------- | ------- |
| 5 | `    return a + b;` | <pre>  %5 = load i32, ptr %3, align 4, !dbg !2896
  %6 = load i32, ptr %4, align 4, !dbg !2897
  %7 = add nsw i32 %5, %6, !dbg !2898
  ret i32 %7, !dbg !2899</pre> |  Loads two 32-bit integers from memory, adds them together with no signed wrap, and returns the result.|
| 10 | `    if (a > b) {` | <pre>  %6 = load i32, ptr %4, align 4, !dbg !2896
  %7 = load i32, ptr %5, align 4, !dbg !2898
  %8 = icmp sgt i32 %6, %7, !dbg !2899
  br i1 %8, label %9, label %11, !dbg !2900</pre> |  Loads two 32-bit` integers from memory, compares them using signed greater-than, and branches based on the result.|
| 11 | `        return a;` | <pre>  %10 = load i32, ptr %4, align 4, !dbg !2901
  store i32 %10, ptr %3, align 4, !dbg !2903
  br label %13, !dbg !2903</pre> |  Loads a 32-bit integer from memory location `%4` into register `%10`, then stores `%10` back into memory location `%3`.|
| 13 | `        return b;` | <pre>  %12 = load i32, ptr %5, align 4, !dbg !2904
  store i32 %12, ptr %3, align 4, !dbg !2906
  br label %13, !dbg !2906</pre> |  Loads a 32-bit integer from memory location `%5` and stores it in memory location `%3`, then branches to label `%13`.|
| 15 | `}` | <pre>  %14 = load i32, ptr %3, align 4, !dbg !2907
  ret i32 %14, !dbg !2907</pre> |  Loads a 32-bit integer from memory address %3 with 4-byte alignment, then returns the loaded value.|
| 19 | `    int result = 0;` | <pre>  store i32 0, ptr %3, align 4, !dbg !2893</pre> |  Stores the 32-bit integer value 0 into memory location pointed to by %3, with 4-byte alignment.|
| 20 | `    for (int i = 1; i <= n; i++) {` | <pre>  store i32 1, ptr %4, align 4, !dbg !2896
  br label %5, !dbg !2897
  %6 = load i32, ptr %4, align 4, !dbg !2898
  %7 = load i32, ptr %2, align 4, !dbg !2900
  %8 = icmp sle i32 %6, %7, !dbg !2901
  br i1 %8, label %9, label %16, !dbg !2902
  %14 = load i32, ptr %4, align 4, !dbg !2907
  %15 = add nsw i32 %14, 1, !dbg !2907
  store i32 %15, ptr %4, align 4, !dbg !2907
  br label %5, !dbg !2908, !llvm.loop !2909</pre> |  Stores initial value 1 into memory, then enters a loop that checks and increments the value until a condition is satisfied.|
| 21 | `        result += i;` | <pre>  %10 = load i32, ptr %4, align 4, !dbg !2903
  %11 = load i32, ptr %3, align 4, !dbg !2905
  %12 = add nsw i32 %11, %10, !dbg !2905
  store i32 %12, ptr %3, align 4, !dbg !2905</pre> |  Loads two 32-bit` integers from memory, adds them together, and stores the result back into memory.|
| 22 | `    }` | <pre>  br label %13, !dbg !2906</pre> |  Unconditional branch to label %13, carrying debug information !2906.|
| 23 | `    return result;` | <pre>  %17 = load i32, ptr %3, align 4, !dbg !2912
  ret i32 %17, !dbg !2913</pre> |  Loads a 32-bit integer value from memory location %3 with 4-byte alignment, then returns that value.|
| 27 | `    int x = 5;` | <pre>  store i32 5, ptr %2, align 4, !dbg !2891</pre> |  Stores the 32-bit integer value 5 into the memory location pointed to, with a 4-byte alignment requirement.|
| 28 | `    int y = 10;` | <pre>  store i32 10, ptr %3, align 4, !dbg !2893</pre> |  Stores the 32-bit integer value 10 into the memory location pointed to by %3, with a 4-byte alignment requirement.|
| 30 | `    std::cout << "Sum: " << add(x, y) << std::endl;` | <pre>  %4 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__1lsB8ne190102INS_11char_traitsIcEEEERNS_13basic_ostreamIcT_EES6_PKc(ptr noundef nonnull align 8 dereferenceable(8) @_ZNSt3__14coutE, ptr noundef @.str), !dbg !2894
  %5 = load i32, ptr %2, align 4, !dbg !2895
  %6 = load i32, ptr %3, align 4, !dbg !2896
  %7 = call i32 @_Z3addii(i32 noundef %5, i32 noundef %6), !dbg !2897
  %8 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEi(ptr noundef %4, i32 noundef %7), !dbg !2898
  %9 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsB8ne190102EPFRS3_S4_E(ptr noundef %8, ptr noundef @_ZNSt3__14endlB8ne190102IcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_), !dbg !2899</pre> |  Calls `_ZNSt3__1ls` with `std::cout` and a string literal, then chains calls to `add`, `basic_ostream::operator<<`, and `basic_ostream::operator<<(std::endl)`.|
| 31 | `    std::cout << "Max: " << max(x, y) << std::endl;` | <pre>  %10 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__1lsB8ne190102INS_11char_traitsIcEEEERNS_13basic_ostreamIcT_EES6_PKc(ptr noundef nonnull align 8 dereferenceable(8) @_ZNSt3__14coutE, ptr noundef @.str.1), !dbg !2900
  %11 = load i32, ptr %2, align 4, !dbg !2901
  %12 = load i32, ptr %3, align 4, !dbg !2902
  %13 = call i32 @_Z3maxii(i32 noundef %11, i32 noundef %12), !dbg !2903
  %14 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEi(ptr noundef %10, i32 noundef %13), !dbg !2904
  %15 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsB8ne190102EPFRS3_S4_E(ptr noundef %14, ptr noundef @_ZNSt3__14endlB8ne190102IcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_), !dbg !2905</pre> |  Here are the explanations:

%10 = ... : Calls std::ostream's insertion operator with a string literal "Max: ".
%11 = ... : Loads an i32 value from memory.
%12 = ... : Loads another i32 value from a different memory location.
%14 = ... : Inserts the result of the max function into the ostream.
%15 = ... : Inserts the endl object into the ostream.|
| 32 | `    std::cout << "Sum of 1 to 10: " << sum(10) << std::endl;` | <pre>  %16 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__1lsB8ne190102INS_11char_traitsIcEEEERNS_13basic_ostreamIcT_EES6_PKc(ptr noundef nonnull align 8 dereferenceable(8) @_ZNSt3__14coutE, ptr noundef @.str.2), !dbg !2906
  %17 = call i32 @_Z3sumi(i32 noundef 10), !dbg !2907
  %18 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsEi(ptr noundef %16, i32 noundef %17), !dbg !2908
  %19 = call noundef nonnull align 8 dereferenceable(8) ptr @_ZNSt3__113basic_ostreamIcNS_11char_traitsIcEEElsB8ne190102EPFRS3_S4_E(ptr noundef %18, ptr noundef @_ZNSt3__14endlB8ne190102IcNS_11char_traitsIcEEEERNS_13basic_ostreamIT_T0_EES7_), !dbg !2909</pre> |  Calls `_ZNSt3__1lsB8ne190102INS_11char_traitsIcEEEERNS_13basic_ostreamIcT_EES6_PKc` to output a string literal, then calls `_Z3sumi` to compute a sum.|
| 34 | `    return 0;` | <pre>  ret i32 0, !dbg !2910</pre> |  Returning an i32 value of 0, with debugging information referenced by !dbg !2910.|
