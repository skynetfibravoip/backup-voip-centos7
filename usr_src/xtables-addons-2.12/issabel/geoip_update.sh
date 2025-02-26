#!/bin/bash
source /etc/geoip_key.conf
function createPL {
read -r -d '' VAR <<'EOF'
#!/usr/bin/perl -w
use strict;
use diagnostics;
use NetAddr::IP;
use Getopt::Long;

my $quiet = 0;
GetOptions(
    'quiet' => \$quiet,
    ) or die("bad args");

unless(-s "$ARGV[0]"){
	print STDERR "Specify Country DB to use on the command line.\n";
	exit 1;
}

# Prime country data with additional continent codes
# http://download.geonames.org/export/dump/readme.txt
my $countryinfo;
$countryinfo->{'6255146'}->{'code'} = 'AF';
$countryinfo->{'6255146'}->{'name'} = 'Africa';
$countryinfo->{'6255147'}->{'code'} = 'AS';
$countryinfo->{'6255147'}->{'name'} = 'Asia';
$countryinfo->{'6255148'}->{'code'} = 'EU';
$countryinfo->{'6255148'}->{'name'} = 'Europe';
$countryinfo->{'6255149'}->{'code'} = 'NA';
$countryinfo->{'6255149'}->{'name'} = 'North America';
$countryinfo->{'6255150'}->{'code'} = 'SA';
$countryinfo->{'6255150'}->{'name'} = 'South America';
$countryinfo->{'6255151'}->{'code'} = 'OC';
$countryinfo->{'6255151'}->{'name'} = 'Oceania';
$countryinfo->{'6255152'}->{'code'} = 'AN';
$countryinfo->{'6255152'}->{'name'} = 'Antarctica';

# Read the countryinfo file
open my $fh_in, "<", "$ARGV[0]" or die "Can't open $ARGV[0]: $!\n";
foreach my $line (<$fh_in>){
	chomp $line;
	next if ($line =~ /^#/);
	my @fields = (split "\t", $line);
	my $code = $fields[0];
	my $name = $fields[4];
	my $id   = $fields[16];
	$countryinfo->{$id}->{'code'} = $code;
	$countryinfo->{$id}->{'name'} = $name;
}
close $fh_in;

# Convert actual GeoLite2 data from STDIN
my $counter;
foreach my $line (<STDIN>){
	next unless ($line =~ /^\d/);
	chomp $line;
	$counter++;
	my @fields = (split ",", $line);
	my $network = $fields[0];
	my $geoname_id = $fields[1];
	my $registered_country_geoname_id = $fields[2];
	my $represented_country_geoname_id = $fields[3];
	my $is_anonymous_proxy = $fields[4];
	my $is_satellite_provider = $fields[5];
	my $ip = NetAddr::IP->new($network);
	#my $start_ip = $ip->canon();
        my $start_ip = _canon($network);
	my $end_ip = $ip->broadcast();
	my $start_int = $ip->bigint();
	my $end_int = $end_ip->bigint();
	my $code;
	my $name;
	if ($is_anonymous_proxy){
		$code = "A1";
		$name = "Anonymous Proxy";
	}elsif ($is_satellite_provider){
		$code = "A2";
		$name = "Satellite Provider";
	}elsif($countryinfo->{$represented_country_geoname_id}){
		$code = $countryinfo->{$represented_country_geoname_id}->{'code'};
		$name = $countryinfo->{$represented_country_geoname_id}->{'name'};
	}elsif($countryinfo->{$registered_country_geoname_id}){
		$code = $countryinfo->{$registered_country_geoname_id}->{'code'};
		$name = $countryinfo->{$registered_country_geoname_id}->{'name'};
	}elsif($countryinfo->{$geoname_id}){
		$code = $countryinfo->{$geoname_id}->{'code'};
		$name = $countryinfo->{$geoname_id}->{'name'};
	}else{
		print STDERR "Unknown Geoname ID, panicking. This is a bug.\n";
		print STDERR "ID: $geoname_id\n";
		print STDERR "ID Registered: $registered_country_geoname_id\n";
		print STDERR "ID Represented $represented_country_geoname_id\n";
		exit 1;
	}

	# Legacy GeoIP listing format:
	# "1.0.0.0","1.0.0.255","16777216","16777471","AU","Australia"
	printf "\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n", 
		$start_ip, _canon($end_ip), $start_int, $end_int, $code, $name;
	if (!$quiet && $counter % 10000 == 0) {
		print STDERR "$counter\n";
	}
}

sub _canon {
    my $network = shift;
    my @partes = split(/\//,$network);
    my @octetos = split(/[:\.]/,$partes[0]);

    if($network =~ m/:/)  {
        # ipv6
        return "ipv6 ".lc(_compV6($partes[0]));
    } else {
        # ipv4
        return $octetos[0].".".$octetos[1].".".$octetos[2].".".$octetos[3];
    }
}

sub _compV6 ($) {
    my $ip = shift;
    return $ip unless my @candidates = $ip =~ /((?:^|:)0(?::0)+(?::|$))/g;
    my $longest = (sort { length($b) <=> length($a) } @candidates)[0];
    $ip =~ s/$longest/::/;
    return $ip;
}

EOF
echo "$VAR" > /usr/share/geoip/20_convert_geolite2
chmod a+x /usr/share/geoip/20_convert_geolite2
}

function fromGeolite2 {
    cd /usr/share/geoip
    TEMPZIP=$(mktemp)
    #LICENSE_KEY=
    GEOLITEURL="https://download.maxmind.com/app/geoip_download?edition_id=GeoLite2-Country-CSV&license_key=$LICENSE_KEY&suffix=zip"
    if ! curl -s $GEOLITEURL > $TEMPZIP
    then
        echo "ERROR - CAN NOT DOWNLOAD DATABASE"
    fi
    if ! unzip -d /tmp -o -j $TEMPZIP '*/GeoLite2-Country-Blocks*'
    then
        echo "ERROR - CAN NOT UNZIP DATABASE"
    fi
    rm $TEMPZIP

    COUNTRYURL='http://download.geonames.org/export/dump/countryInfo.txt'
    if ! curl -s $COUNTRYURL > /tmp/CountryInfo.txt
    then
        echo "ERROR - CAN NOT DOWNLOAD GEONAMES"
    fi

    cat /tmp/GeoLite2-Country-Blocks-IPv{4,6}.csv | ./20_convert_geolite2 /tmp/CountryInfo.txt > GeoIP-legacy.csv

    ./xt_geoip_build GeoIP-legacy.csv
}

cd /usr/share/geoip
mkdir -p /usr/share/xt_geoip

if [ ! -f /usr/share/geoip/20_convert_geolite2 ]
then
    createPL
fi

fromGeolite2

alias cp=cp
cp -rf {BE,LE} /usr/share/xt_geoip
alias cp='cp -i'
