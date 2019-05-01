clear && gcc -Wall -Werror -std=c99 -pthread dbms.c -o d && ./d
for i in {1..7}
do
    ./d < "input$i.txt" > "test$i.txt";
done
