require './waptools.so'

$interface = ARGV[0]

usage = Proc.new { puts "Usage: ruby #{__FILE__} [wlan interface]"; exit }
usage.call unless !$interface.nil?

puts "#{WAPTools::authors}\n\n"

scanner = WAPTools::Scanner.new($interface)
aps = scanner.scan

aps.each { |ap|
#   Note: ap.to_s will output all details of the nearby AP
#	puts ap.to_s

        next if ap.nil?

	puts "SSID     : #{ap.ssid}"
	puts "BSSID    : #{ap.bssid}"	
	puts "Frequency: #{ap.frequency}"
	puts "Bitrate  : #{ap.bitrate}"
	puts "Stats    : #{ap.stats}"
	puts "Channel  : #{ap.channel}"
	puts
}
