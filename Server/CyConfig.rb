require './DefaultConfig.rb'

class CyConfig
  def check
    check_acct
    check_data_dir
  end

  def remove_all
    remove_cypress_acct
    remove_data_dir
  end

  def get_new_password
    a = "a"
    b = "b"
    while a != b
      print "cypress password: " 
      STDOUT.flush
      a = STDIN.noecho(&:gets)
      puts ""

      print "again: " 
      STDOUT.flush
      b = STDIN.noecho(&:gets)
      puts ""

      puts "Passowrds do not match" unless a == b
    end
    a
  end

  def create_cypress_acct
      if RUBY_PLATFORM.include? "linux"
        puts "creating Linux user cypress"
        p = get_new_password
        `sudo useradd -m -p #{p.crypt(SecureRandom.hex[0..1])} cypress`
        `sudo mkdir /home/cypress`
        `sudo chown cypress /home/cypress`
      elsif RUBY_PLATFORM.include? "darwin"
        puts "creating OSX user cypress"
        `sudo dscl . -create /Users/cypress`
        `sudo dscl . -create /Users/cypress UserShell /bin/bash`
        `sudo dscl . -create /Users/cypress UniqueID 547`
        `sudo dscl . -create /Users/cypress PrimaryGroupID 1000`
        `sudo mkdir /Users/cypress`
        `sudo chown cypress /Users/cypress`
        `sudo dscl . -create /Users/cypress NFSHomeDirectory /Users/cypress`
        p = get_new_password
        `sudo dscl . -passwd /Users/cypress #{p}`
      end
  end

  def remove_cypress_acct
      if RUBY_PLATFORM.include? "linux"
        `sudo rm -rf /home/cypress`
      elsif RUBY_PLATFORM.include? "darwin"
        `sudo dscl . -delete /Users/cypress`
        `sudo rm -rf /Users/cypress`
      end
  end

  def check_acct
    begin
     Process.uid != Process::UID.from_name("cypress")
    rescue
      puts "cypress user account does not exist, creating it"
      create_cypress_acct
    end
  end

  def check_data_dir
    ddir = Cypress::ServerConfig::DATA[:base_dir]
    if not Dir.exists? ddir 
      puts "Data directory #{ddir} does not exist, creating it"
      `sudo mkdir -p #{ddir}`
      `sudo chown cypress #{ddir}`
    end
  end

  def remove_data_dir
    ddir = Cypress::ServerConfig::DATA[:base_dir]
    `sudo rm -rf #{ddir}`
  end

end
