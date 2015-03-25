//cypress node.js client

var request = require('request');

var usage = 
"usage: cyp <subcommand> \n" +
"  subcommand := \n" +
"    design \n" +
"    control \n" +
"    data \n";

if(process.argv.length < 3) {
  console.log(usage);
  process.exit(1);
}

var subcommand = process.argv[2];

if(subcommand === 'design')
{
  usage = 
  "usage: cyp design <subcommand> \n" +
  "  subcommand := \n" +
  "    new \n" +
  "    sync \n" +
  "    checkpoint \n" +
  "    compile \n";

  if(process.argv.length < 4) {
    console.log(usage);
    process.exit(1);
  }

  subcommand = process.argv[3];

  if(subcommand === 'new')
  {
    if(process.argv.length < 5) {
      var usage = "usage: cyp design new <name> \n";
      console.log(usage);
      process.exit(1);
    }
    var expname = process.argv[4];
    request.post(
        'http://localhost:4567/Design/New',
        { form: { name: expname } },
        function(error, response, body) {
          if(!error && response.statusCode == 200) {
            console.log(body)
          }
        });
  }
  else if(subcommand === 'sync') {
    console.log('cyp design sync - not implemented yet');
    process.exit(1);
  }
  else if(subcommand === 'checkpoint') {
    console.log('cyp design checkpoint - not implemented yet');
    process.exit(1);
  }
  else if(subcommand === 'compile') {
    console.log('cyp design compile - not implemented yet');
    process.exit(1);
  }
  else { 
    console.log('cyp design - unkown subcommand "'+subcommand+'"');
    console.log(usage);
    process.exit(1);
  }
}

else if(subcommand === 'control')
{
  console.log('cyp control - not implemented yet');
  process.exit(1);
}
else if(subcommand === 'data')
{
  console.log('cyp data - not implemented yet');
  process.exit(1);
}

else{ console.log(usage); }
