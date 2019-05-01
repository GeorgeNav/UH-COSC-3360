import 'dart:io';

main() {
  Process.start('gcc', ['dbms.c', '-o', 'd'], runInShell: true).then((process) {

  });
  Process.start('./d', ['<', 'input1'], runInShell: true).then((process) {
      stdout.addStream(process.stdout);
      stderr.addStream(process.stderr);
  });

/*   for(int i = 1; i < 8; i++)
    Process.start('./d', ['<input$i'], runInShell: true).then((process) {
        stdout.addStream(process.stdout);
        stderr.addStream(process.stderr);
    }); */
}