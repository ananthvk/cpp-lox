1) There is a bug with switch-case in the following code, the created new variable inside the loop
never gets popped, leading to stack overflow.

status: unfixed

```
input();
for(var i = 0; i < 1000001; i = i + 1) {
    var a = "haa" + to_string(i);
    switch(i) {
        case 10000: echo "10 000";
        case 50000: echo "50 000";
        case 100000: echo "100 000";
        case 200000: echo "200 000";
        case 300000: echo "300 000";
        case 400000: echo "400 000";
        case 500000: echo "500 000";
        case 600000: echo "600 000";
        case 700000: echo "700 000";
        case 800000: echo "800 000";
        case 900000: echo "900 000";
    }
    if(i == 1000000)
     {
         echo "done";
         input();
     }
}
```