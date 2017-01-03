#!/usr/bin/perl

my $OCTAVE_OFS=3;
my $DURATION_SCALE=250;

my $start=-1;
my $curnote=-1;
my $curoctave=-1;

my @song=();

#
#
#
# 1 -> 1/2 1/4 1/8 1/16 1/32
#

my @NOTES=('C','C#','D','D#','E','F','F#','G','G#','A','A#','B');

while ($_ = <>) {
    if (/\d+,\s*(\d+),\s*Note_(\w+)_c,\s*\d+,\s*(\d+),\s*(\d+)/) {
	my $time=$1;
	my $onoff=$2;
	my $midinote=$3;
	my $velo=$4;
	$velo=0 if ($onoff eq "off");
	
	if ($start!=-1) {
	    my $duration=$time-$start;
	    my $len=int(($duration/$DURATION_SCALE)+0.5)-1;

	    if ($curnote==-1) { # pause
		for(;($len>15);$len-=16) {
		    printf("0xfd,\t/* 16 pause */\n");
		}
		if ($len>=0) {
		    printf("0x%02x,\t/* %d pause */\n",$len<<4|0x0d,$len+1);
		}
		      
	    } else {
		my $octave=int($curnote/12)-$OCTAVE_OFS;
		die "Wrong octave: $octave\n" if (($octave<0) || ($octave>3));
		my $note=$curnote % 12;
	    
		if ($len>3) {
		    printf("0x%02x,\t/* %d%s (4) [%d] */\n",
			(($octave<<6)|0x30|$note)&0xff,$octave,$NOTES[$note],$len+1);
		    for($len-=4;($len>15);$len-=16) {
			printf("0xfe,\t/* extended 16 counts */\n");
		    }
		    if ($len>=0) {
			printf("0x%02x,\t/* extended %d counts */\n",
			    $len<<4|0x0e,$len+1);
		    }
		} else {
		    printf("0x%02x,\t/* %d%s (%d) */\n",
			(($octave<<6)|($len<<4)|$note)&0xff,$octave,$NOTES[$note],$len+1);
		}
	    }
	}
	$start=$time;
	$curnote=$midinote;
	$curnote=-1 if $velo==0;
    }
}
    


	