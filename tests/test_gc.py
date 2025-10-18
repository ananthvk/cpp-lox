# Tests the GC
# Note: The tests are run with --stress-gc flag, that causes the GC to run for every allocation
# If this flag is removed when running tests, some tests may fail since the gc would not have collected
def test_gc_memory_allocation_tracking(run_lox):
    assert (
        run_lox(
            """
    var initial_allocated = sys__mem_get_bytes_allocated();
    var initial_objects = sys__mem_get_objects_created();
    
    fun allocate() {
        var str1 = to_string(42);
        var str2 = to_string(123);
        return to_string(999);
    }
    allocate();
    
    var final_allocated = sys__mem_get_bytes_allocated();
    var final_objects = sys__mem_get_objects_created();
    
    echo final_allocated >= initial_allocated;
    echo final_objects > initial_objects;
    """
        )
        == "true\ntrue"
    )


def test_gc_object_creation_and_cleanup(run_lox):
    assert (
        run_lox(
            """
    var objects_before = sys__mem_get_objects_created();
    var live_before = sys__mem_get_live_objects();
    
    fun createTemporary() {
        var temp1 = to_string(1);
        var temp2 = to_string(2);
        var temp3 = to_string(3);
    }
    createTemporary();
    
    var objects_after = sys__mem_get_objects_created();
    var live_after = sys__mem_get_live_objects();
    
    echo objects_after > objects_before;
    echo live_after >= live_before;
    """
        )
        == "true\ntrue"
    )


def test_gc_net_bytes_consistency(run_lox):
    assert (
        run_lox(
            """
    var allocated = sys__mem_get_bytes_allocated();
    var freed = sys__mem_get_bytes_freed();
    var net = sys__mem_get_net_bytes();
    
    echo net == (allocated - freed);
    echo net >= 0;
    """
        )
        == "true\ntrue"
    )


def test_gc_threshold_tracking(run_lox):
    assert (
        run_lox(
            """
    var threshold = sys__mem_get_next_gc();
    var current_net = sys__mem_get_net_bytes();
    
    echo threshold > 0;
    echo threshold >= current_net;
    """
        )
        == "true\ntrue"
    )


def test_gc_closure_memory_preservation(run_lox):
    assert (
        run_lox(
            """
    var live_before = sys__mem_get_live_objects();
    var globalFunc;
    
    fun outer() {
        var preserved = to_string(555);
        fun inner() {
            echo preserved;
        }
        globalFunc = inner;
    }
    outer();
    
    var live_after = sys__mem_get_live_objects();
    globalFunc();
    
    // Should have more live objects due to closure
    echo live_after >= live_before;
    """
        )
        == "555\ntrue"
    )


def test_gc_stress_allocation(run_lox):
    assert (
        run_lox(
            """
    var initial_objects = sys__mem_get_objects_created();
    var initial_allocated = sys__mem_get_bytes_allocated();
    
    fun stressTest() {
        for (var i = 0; i < 100; i = i + 1) {
            var temp = to_string(i);
        }
    }
    stressTest();
    
    var final_objects = sys__mem_get_objects_created();
    var final_allocated = sys__mem_get_bytes_allocated();
    
    echo final_objects > initial_objects;
    echo final_allocated > initial_allocated;
    """
        )
        == "true\ntrue"
    )


def test_gc_nested_closure_memory(run_lox):
    assert (
        run_lox(
            """
    var live_before = sys__mem_get_live_objects();
    var outerClosure;
    
    fun level1() {
        var data1 = to_string(111);
        fun level2() {
            var data2 = to_string(222);
            fun level3() {
                echo data1;
                echo data2;
            }
            return level3;
        }
        outerClosure = level2();
    }
    level1();
    
    var live_after = sys__mem_get_live_objects();
    outerClosure();
    
    echo live_after > live_before;
    """
        )
        == "111\n222\ntrue"
    )


def test_gc_freed_bytes_tracking(run_lox):
    assert (
        run_lox(
            """
    var freed_before = sys__mem_get_bytes_freed();
    
    fun allocateAndDiscard() {
        var temp1 = to_string(777);
        var temp2 = to_string(888);
    }
    allocateAndDiscard();
    
    var freed_after = sys__mem_get_bytes_freed();
    
    echo freed_after >= freed_before;
    echo sys__mem_get_net_bytes() >= 0;
    """
        )
        == "true\ntrue"
    )


def test_gc_live_objects_stability(run_lox):
    assert (
        run_lox(
            """
    var persistent = to_string(333);
    var live_initial = sys__mem_get_live_objects();
    
    fun temporaryWork() {
        var temp1 = to_string(444);
        var temp2 = to_string(555);
        echo temp1;
    }
    temporaryWork();
    
    var live_final = sys__mem_get_live_objects();
    echo persistent;
    
    echo live_final >= live_initial;
    """
        )
        == "444\n333\ntrue"
    )


def test_gc_allocation_growth_pattern(run_lox):
    assert (
        run_lox(
            """
    var allocated1 = sys__mem_get_bytes_allocated();
    var str1 = to_string(1);
    
    var allocated2 = sys__mem_get_bytes_allocated();
    var str2 = to_string(22);
    
    var allocated3 = sys__mem_get_bytes_allocated();
    var str3 = to_string(333);
    
    echo allocated2 > allocated1;
    echo allocated3 > allocated2;
    echo sys__mem_get_net_bytes() > 0;
    """
        )
        == "true\ntrue\ntrue"
    )


def test_gc_recursive_function_memory(run_lox):
    assert (
        run_lox(
            """
    var initial_objects = sys__mem_get_objects_created();
    var initial_net = sys__mem_get_net_bytes();
    
    fun fibonacci(n) {
        if (n <= 1) return n;
        var temp1 = to_string(n);
        var result = fibonacci(n - 1) + fibonacci(n - 2);
        return result;
    }
    
    var fib_result = fibonacci(8);
    var final_objects = sys__mem_get_objects_created();
    var final_net = sys__mem_get_net_bytes();
    
    echo final_objects > initial_objects;
    echo final_net >= initial_net;
    echo fib_result == 21;
    """
        )
        == "true\ntrue\ntrue"
    )


def test_gc_multiple_closure_chains(run_lox):
    assert (
        run_lox(
            """
    var live_before = sys__mem_get_live_objects();
    var chain1;
    var chain2;
    
    fun createChain(id) {
        var chainId = to_string(id);
        fun step1() {
            var data1 = to_string(100 + id);
            fun step2() {
                var data2 = to_string(200 + id);
                fun step3() {
                    echo chainId;
                    echo data1;
                    echo data2;
                }
                return step3;
            }
            return step2();
        }
        return step1();
    }
    
    chain1 = createChain(1);
    chain2 = createChain(2);
    
    var live_after = sys__mem_get_live_objects();
    
    chain1();
    chain2();
    
    echo live_after > live_before;
    """
        )
        == "1\n101\n201\n2\n102\n202\ntrue"
    )


def test_gc_memory_pressure_with_calculations(run_lox):
    assert (
        run_lox(
            """
    var allocated_start = sys__mem_get_bytes_allocated();
    var freed_start = sys__mem_get_bytes_freed();
    var total_allocations = 0;
    
    fun intensiveCalculation() {
        for (var i = 1; i <= 50; i = i + 1) {
            var str_i = to_string(i);
            var str_double = to_string(i * 2);
            var str_square = to_string(i * i);
            total_allocations = total_allocations + 3;
        }
    }
    
    intensiveCalculation();
    
    var allocated_end = sys__mem_get_bytes_allocated();
    var freed_end = sys__mem_get_bytes_freed();
    var objects_created = sys__mem_get_objects_created();
    
    echo allocated_end > allocated_start;
    echo freed_end >= freed_start;
    echo total_allocations == 150;
    echo objects_created > 0;
    """
        )
        == "true\ntrue\ntrue\ntrue"
    )


def test_gc_closure_variable_capture_patterns(run_lox):
    assert (
        run_lox(
            """
    var live_initial = sys__mem_get_live_objects();
    var captureFunc;
    
    fun complexCapture() {
        var outer_var = to_string(777);
        var counter = 0;
        
        fun incrementer() {
            counter = counter + 1;
            var counter_str = to_string(counter);
            
            fun displayer() {
                echo outer_var;
                echo counter_str;
                echo counter;
            }
            return displayer;
        }
        
        captureFunc = incrementer();
    }
    
    complexCapture();
    var live_after_creation = sys__mem_get_live_objects();
    
    captureFunc();
    var net_bytes = sys__mem_get_net_bytes();
    
    echo live_after_creation > live_initial;
    echo net_bytes > 0;
    """
        )
        == "777\n1\n1\ntrue\ntrue"
    )


def test_gc_memory_fragmentation_simulation(run_lox):
    assert (
        run_lox(
            """
    var initial_net = sys__mem_get_net_bytes();
    var initial_threshold = sys__mem_get_next_gc();
    var phase1_allocations = 0;
    var phase2_allocations = 0;
    
    fun phase1() {
        for (var i = 1; i <= 30; i = i + 1) {
            var small_str = to_string(i);
            phase1_allocations = phase1_allocations + 1;
        }
    }
    
    fun phase2() {
        for (var j = 100; j <= 130; j = j + 1) {
            var medium_str = to_string(j * 10);
            var large_str = to_string(j * j);
            phase2_allocations = phase2_allocations + 2;
        }
    }
    
    phase1();
    var mid_net = sys__mem_get_net_bytes();
    
    phase2();
    var final_net = sys__mem_get_net_bytes();
    var final_threshold = sys__mem_get_next_gc();
    
    echo phase1_allocations == 30;
    echo phase2_allocations == 62;
    echo final_net >= mid_net;
    echo final_net > initial_net;
    echo final_threshold > 0;
    """
        )
        == "true\ntrue\ntrue\ntrue\ntrue"
    )