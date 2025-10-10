def test_basic_local_variables(run_lox):
    assert (
        run_lox(
            """
    {
        var x = 10;
        echo x;
    }
    """
        )
        == "10"
    )


def test_nested_scope_shadowing(run_lox):
    assert (
        run_lox(
            """
    var a = "global";
    {
        var a = "local";
        echo a;
    }
    echo a;
    """
        )
        == """local
global"""
    )


def test_multiple_variables_same_scope(run_lox):
    assert (
        run_lox(
            """
    {
        var x = 1;
        var y = 2;
        var z = 3;
        echo x;
        echo y;
        echo z;
    }
    """
        )
        == """1
2
3"""
    )


def test_variable_reassignment_in_scope(run_lox):
    assert (
        run_lox(
            """
    {
        var a = 5;
        echo a;
        a = 10;
        echo a;
        a = 15;
        echo a;
    }
    """
        )
        == """5
10
15"""
    )


def test_deep_nesting_access(run_lox):
    assert (
        run_lox(
            """
    var outer = "outer";
    {
        var middle = "middle";
        {
            var inner = "inner";
            echo outer;
            echo middle;
            echo inner;
        }
    }
    """
        )
        == """outer
middle
inner"""
    )


def test_scope_isolation(run_lox):
    assert (
        run_lox(
            """
    {
        var a = 1;
    }
    {
        var a = 2;
        echo a;
    }
    """
        )
        == "2"
    )


def test_mixed_global_local_operations(run_lox):
    assert (
        run_lox(
            """
    var global_var = 100;
    {
        echo global_var;
        var local_var = 200;
        global_var = 300;
        echo global_var;
        echo local_var;
    }
    echo global_var;
    """
        )
        == """100
300
200
300"""
    )


def test_variable_scope_with_expressions(run_lox):
    assert (
        run_lox(
            """
    var x = 5;
    {
        var y = x + 3;
        echo y;
        x = y * 2;
        echo x;
    }
    echo x;
    """
        )
        == """8
16
16"""
    )


def test_sequential_scopes_with_same_names(run_lox):
    assert (
        run_lox(
            """
    {
        var temp = 1;
        echo temp;
    }
    {
        var temp = 2;
        echo temp;
    }
    {
        var temp = 3;
        echo temp;
    }
    """
        )
        == """1
2
3"""
    )


def test_complex_nested_shadowing(run_lox):
    assert (
        run_lox(
            """
    var name = "global";
    {
        var name = "outer";
        echo name;
        {
            echo name;
            var name = "inner";
            echo name;
            {
                var name = "deepest";
                echo name;
            }
            echo name;
        }
        echo name;
    }
    echo name;
    """
        )
        == """outer
outer
inner
deepest
inner
outer
global"""
    )


def test_local_global_shadowing(run_lox):
    assert (
        run_lox(
            """
    var outer1 = "hello world";
    var outer2 = "bye world";
    {
        var outer1 = "inside scope";
        echo outer1;
        {
            outer1 = "deep";
            echo outer2;
        }
        var outer2 = "inside scope";
        echo outer1;
    }
    echo outer1;
    echo outer2;
                   """
        )
        == """inside scope
bye world
deep
hello world
bye world"""
    )


def test_many_locals_256_variables(run_lox):
    # Test with exactly 256 local variables across different scopes
    code = """
    {
        // First scope with 64 variables
        var v1 = 1; var v2 = 2; var v3 = 3; var v4 = 4;
        var v5 = 5; var v6 = 6; var v7 = 7; var v8 = 8;
        var v9 = 9; var v10 = 10; var v11 = 11; var v12 = 12;
        var v13 = 13; var v14 = 14; var v15 = 15; var v16 = 16;
        var v17 = 17; var v18 = 18; var v19 = 19; var v20 = 20;
        var v21 = 21; var v22 = 22; var v23 = 23; var v24 = 24;
        var v25 = 25; var v26 = 26; var v27 = 27; var v28 = 28;
        var v29 = 29; var v30 = 30; var v31 = 31; var v32 = 32;
        var v33 = 33; var v34 = 34; var v35 = 35; var v36 = 36;
        var v37 = 37; var v38 = 38; var v39 = 39; var v40 = 40;
        var v41 = 41; var v42 = 42; var v43 = 43; var v44 = 44;
        var v45 = 45; var v46 = 46; var v47 = 47; var v48 = 48;
        var v49 = 49; var v50 = 50; var v51 = 51; var v52 = 52;
        var v53 = 53; var v54 = 54; var v55 = 55; var v56 = 56;
        var v57 = 57; var v58 = 58; var v59 = 59; var v60 = 60;
        var v61 = 61; var v62 = 62; var v63 = 63; var v64 = 64;
        
        {
            // Second scope with 64 more variables (128 total)
            var w1 = 65; var w2 = 66; var w3 = 67; var w4 = 68;
            var w5 = 69; var w6 = 70; var w7 = 71; var w8 = 72;
            var w9 = 73; var w10 = 74; var w11 = 75; var w12 = 76;
            var w13 = 77; var w14 = 78; var w15 = 79; var w16 = 80;
            var w17 = 81; var w18 = 82; var w19 = 83; var w20 = 84;
            var w21 = 85; var w22 = 86; var w23 = 87; var w24 = 88;
            var w25 = 89; var w26 = 90; var w27 = 91; var w28 = 92;
            var w29 = 93; var w30 = 94; var w31 = 95; var w32 = 96;
            var w33 = 97; var w34 = 98; var w35 = 99; var w36 = 100;
            var w37 = 101; var w38 = 102; var w39 = 103; var w40 = 104;
            var w41 = 105; var w42 = 106; var w43 = 107; var w44 = 108;
            var w45 = 109; var w46 = 110; var w47 = 111; var w48 = 112;
            var w49 = 113; var w50 = 114; var w51 = 115; var w52 = 116;
            var w53 = 117; var w54 = 118; var w55 = 119; var w56 = 120;
            var w57 = 121; var w58 = 122; var w59 = 123; var w60 = 124;
            var w61 = 125; var w62 = 126; var w63 = 127; var w64 = 128;
            
            {
                // Third scope with 64 more variables (192 total)
                var x1 = 129; var x2 = 130; var x3 = 131; var x4 = 132;
                var x5 = 133; var x6 = 134; var x7 = 135; var x8 = 136;
                var x9 = 137; var x10 = 138; var x11 = 139; var x12 = 140;
                var x13 = 141; var x14 = 142; var x15 = 143; var x16 = 144;
                var x17 = 145; var x18 = 146; var x19 = 147; var x20 = 148;
                var x21 = 149; var x22 = 150; var x23 = 151; var x24 = 152;
                var x25 = 153; var x26 = 154; var x27 = 155; var x28 = 156;
                var x29 = 157; var x30 = 158; var x31 = 159; var x32 = 160;
                var x33 = 161; var x34 = 162; var x35 = 163; var x36 = 164;
                var x37 = 165; var x38 = 166; var x39 = 167; var x40 = 168;
                var x41 = 169; var x42 = 170; var x43 = 171; var x44 = 172;
                var x45 = 173; var x46 = 174; var x47 = 175; var x48 = 176;
                var x49 = 177; var x50 = 178; var x51 = 179; var x52 = 180;
                var x53 = 181; var x54 = 182; var x55 = 183; var x56 = 184;
                var x57 = 185; var x58 = 186; var x59 = 187; var x60 = 188;
                var x61 = 189; var x62 = 190; var x63 = 191; var x64 = 192;
                
                {
                    // Fourth scope with final 64 variables (256 total)
                    var y1 = 193; var y2 = 194; var y3 = 195; var y4 = 196;
                    var y5 = 197; var y6 = 198; var y7 = 199; var y8 = 200;
                    var y9 = 201; var y10 = 202; var y11 = 203; var y12 = 204;
                    var y13 = 205; var y14 = 206; var y15 = 207; var y16 = 208;
                    var y17 = 209; var y18 = 210; var y19 = 211; var y20 = 212;
                    var y21 = 213; var y22 = 214; var y23 = 215; var y24 = 216;
                    var y25 = 217; var y26 = 218; var y27 = 219; var y28 = 220;
                    var y29 = 221; var y30 = 222; var y31 = 223; var y32 = 224;
                    var y33 = 225; var y34 = 226; var y35 = 227; var y36 = 228;
                    var y37 = 229; var y38 = 230; var y39 = 231; var y40 = 232;
                    var y41 = 233; var y42 = 234; var y43 = 235; var y44 = 236;
                    var y45 = 237; var y46 = 238; var y47 = 239; var y48 = 240;
                    var y49 = 241; var y50 = 242; var y51 = 243; var y52 = 244;
                    var y53 = 245; var y54 = 246; var y55 = 247; var y56 = 248;
                    var y57 = 249; var y58 = 250; var y59 = 251; var y60 = 252;
                    var y61 = 253; var y62 = 254; var y63 = 255; var y64 = 256;
                    
                    // Test that all variables are accessible
                    echo v1;
                    echo w32;
                    echo x64;
                    echo y64;
                }
            }
        }
    }
    """
    assert (
        run_lox(code)
        == """1
96
192
256"""
    )


def test_global_complex(run_lox):
    assert (
        run_lox(
            """
    {
        var a = 1;
        echo a;
        {
            echo a;
            var b = 2;
            {
                var c = 3;
                echo b;
                echo c;
                {
                    var d = 4;
                    echo d;
                }
                var e = 5;
                echo e;
                e = c;
                echo e;
                var b = 20;
                echo b;
            }
            echo b;
            var a = 95;
            echo a;
            a = 100;
            echo a;
        }
        echo a;
        var f = 6;
        echo f;
        {
            var g = 7;
            echo g;
            f = 70;
            echo f;
        }
        echo f;
    }
    """
        )
        == """1
1
2
3
4
5
3
20
2
95
100
1
6
7
70
70"""
    )
