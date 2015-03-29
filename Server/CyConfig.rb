#require './DefaultConfig.rb'
require 'securerandom'
require 'io/console'

class CyConfig

  def initialize(config)
    @config = config
    require @config
    @C = Cypress::ServerConfig

    @required_dirs = [:data, :runtime, :log, :www, :etc, :lib]
    @dir_names = {
      :data => "Data", 
      :runtime => "Runtime",
      :log => "Log",
      :www => "Web",
      :etc => "Config",
      :lib => "Library" }
  end

  def check
    check_acct
    check_dirs
    check_config
  end

  def check_dirs
    @required_dirs.each do |x|
      check_dir(x, @dir_names[x])
    end
  end

  def remove_all
    remove_cypress_acct
    remove_dirs
  end

  def remove_dirs
    @required_dirs.each do |x|
      remove_dir(x)
    end
  end

  def check_config
    check_file("#{@C::DIRS[:etc]}/server", @config)
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

  def check_dir(id, name)
    dir = Cypress::ServerConfig::DIRS[id]
    if not Dir.exists? dir 
      puts "The #{name} directory: #{dir} does not exist, creating it"
      `sudo mkdir -p #{dir}`
      `sudo chown cypress #{dir}`
    end
  end

  def remove_dir(id)
    dir = Cypress::ServerConfig::DIRS[id]
    `sudo rm -rf #{dir}`
  end

  def check_file(path, source)
    if not File.exists? path
      puts "The file #{path} does not exist, copying from #{source}"
      `sudo cp #{source} #{path}`
    end
  end

  def remove_file(path)
    `sudo rm -f #{path}`
  end

end
