/* vobcopy 1.0.x
 *
 * Copyright (c) 2001 - 2006 robos@muon.de
 * Lots of contribution and cleanup from rosenauer@users.sourceforge.net
 * Critical bug-fix from Bas van den Heuvel
 * Takeshi HIYAMA made lots of changes to get it to run on FreeBSD
 * Erik Hovland made changes for solaris
 *  This file is part of vobcopy.
 *
 *  vobcopy is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  vobcopy is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with vobcopy; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

/* CONTRIBUTORS (direct or through source "borrowing") 
 * rosenauer@users.sf.net - helped me a lot! 
 * Billy Biggs <vektor@dumbterm.net> - took some of his play_title.c code 
 * and implemeted it here 
 * H�kan Hjort <d95hjort@dtek.chalmers.se> and Billy Biggs - libdvdread
 */



/*TODO:
 * mnttab reading is "wrong" on solaris
 * - error handling with the errno and strerror function
 * with -t still numbers get added to the name, -T with no numbers or so.
 */

/*THOUGHT-ABOUT FEATURES
 * - being able to specify
     - which title
     - which chapter
     - which angle 
     - which language(s)
     - which subtitle(s)
     to copy
 * - print the total playing time of that title
 */

/*If you find a bug, contact me at robos@muon.de*/

#include "vobcopy.h"

extern int errno;
  char              name[300];
  bool              overwrite_flag = FALSE;
/* --------------------------------------------------------------------------*/
/* MAIN */
/* --------------------------------------------------------------------------*/

int main ( int argc, char *argv[] )
{
  int               streamout, block_count, blocks, file_block_count;
//  char              name[300], op;
  char              op;
  char              dvd_path[255], logfile_name[20],logfile_path[255];
  char              dvd_name[33], vobcopy_call[255], provided_dvd_name[33];
  char              *size_suffix;
  char              pwd[255],provided_output_dir[255],provided_input_dir[255];
  char              alternate_output_dir[4][255], onefile[255];
  unsigned char     bufferin[ DVD_VIDEO_LB_LEN * BLOCK_COUNT ];
  int               i = 0,j = 0, l = 0, argc_i = 0, alternate_dir_count = 0;
  int               partcount = 0, get_dvd_name_return, options_char = 0;
  int               dvd_count = 0, verbosity_level = 0, paths_taken = 0, fast_factor = 1;
  long long unsigned int          seek_start = 0, stop_before_end = 0, temp_var;
  off_t             pwd_free, vob_size = 0, disk_vob_size = 0;
  off_t             offset = 0, free_space = 0; 
  off_t             max_filesize_in_blocks = 1048571;  /* for 2^31 / 2048 */
  off_t             max_filesize_in_blocks_summed = 0, angle_blocks_skipped = 0;
  ssize_t           file_size_in_blocks = 0;
  bool              mounted = FALSE, provided_output_dir_flag = FALSE;
  bool              verbose_flag = FALSE, provided_input_dir_flag = FALSE;
  bool              force_flag = FALSE, info_flag = FALSE, cut_flag = FALSE;
  bool              large_file_flag = FALSE, titleid_flag = FALSE;
  bool              mirror_flag = FALSE, provided_dvd_name_flag = FALSE;
  bool              stdout_flag = FALSE, space_greater_2gb_flag = TRUE;
  bool              fast_switch = FALSE, onefile_flag = FALSE;
  bool              quiet_flag = FALSE;
  struct stat       buf;

  dvd_reader_t      *dvd = NULL;
  dvd_file_t        *dvd_file = NULL;
  extern char       *optarg;
  extern int        optind, optopt;

  /**
   *this is taken from play_title.c
   */
  int               titleid = 2, chapid = 0, pgc_id, start_cell;
  int               angle = 0, ttn, pgn, sum_chapters = 0;
  int               sum_angles = 0, most_chapters = 0;
  ifo_handle_t *vmg_file;
  tt_srpt_t *tt_srpt;
  ifo_handle_t *vts_file;
  vts_ptt_srpt_t *vts_ptt_srpt;
  pgc_t *cur_pgc;

  /*
    this is for the mirror feature (readdir)
  */
  struct dirent * directory;
  DIR *dir;
  
  /**
   *getopt-long
   */
#ifdef HAVE_GETOPT_LONG
  int option_index = 0;
  static struct option long_options[] = {
                   {"1st_alt_output_dir", 1, 0, '1'},
                   {"2st_alt_output_dir", 1, 0, '2'},
                   {"3st_alt_output_dir", 1, 0, '3'},
                   {"4st_alt_output_dir", 1, 0, '4'},
                   {"angle", 1, 0, 'a'},
                   {"begin", 1, 0, 'b'},
                   {"chapter", 1, 0, 'c'},
                   {"end", 1, 0, 'e'},
                   {"force", 0, 0, 'f'},
                   {"fast", 1, 0, 'F'},
                   {"help", 0, 0, 'h'},
                   {"input-dir", 1, 0, 'i'},
                   {"info", 0, 0, 'I'},
                   {"large-file", 0, 0, 'l'},
                   {"mirror", 0, 0, 'm'},
                   {"title-number", 1, 0, 'n'},
                   {"output-dir", 1, 0, 'o'},
                   {"quiet", 0, 0, 'q'},
                   {"onefile", 1, 0, 'O'},
                   {"name", 1, 0, 't'},
                   {"verbose", 0, 0, 'v'},
                   {"version", 0, 0, 'V'},
/*                    {"test", 1, 0, 'test'}, */
                   {0, 0, 0, 0}
  };
#endif
  
  

  /*
   * the getopt part (getting the options from command line)
   */
  while (1)
    {
#ifdef HAVE_GETOPT_LONG
      options_char = getopt_long( argc, argv, 
                                 "1:2:3:4:a:b:c:e:i:n:o:qO:t:vfF:lmhL:VI",
				  long_options ,&option_index);
#else
      options_char = getopt( argc, argv, "1:2:3:4:a:b:c:e:i:n:o:qO:t:vfF:lmhL:VI-" );
#endif

      if ( options_char == -1 ) break;
      
      switch( options_char )
	{
/* 	  debug */
/* 	  case'test': */
/* 	    fprintf( stderr, "%s aufruf %s\n", long_options[option_index].name ,optarg ); */
/* 	  exit(0); */
/* 	  break; */
	  
	  case'a': /*angle*/
	    if ( !isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Error] The thing behind -a has to be a number! \n" );
		exit(1); 
	      } 
	  sscanf( optarg, "%i", &angle );
	  angle--;/*in the ifo they start at zero */
	  if (angle < 0)
	    {
	      fprintf( stderr, "[Hint] Um, you set angle to 0, try 1 instead ;-)\n" );
	      exit(1);
	    }
	  break;

	  case'b': /*size to skip from the beginning (beginning-offset) */
	    if ( !isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Error] The thing behind -b has to be a number! \n" );
		exit(1); 
	      } 
	  temp_var = atol( optarg );
	  size_suffix = strpbrk( optarg, "bkmgBKMG" );
	  if( !size_suffix)
	  {
	  	  fprintf( stderr, "[Error] Wrong suffix behind -b, only b,k,m or g \n" );
	      exit(1); 
	  }

	  switch( *size_suffix )
	    {
	      case'b':
	      case'B':	      
		temp_var *= 512;/*blocks (normal, not dvd)*/
	      break;
	      case'k':
	      case'K':	      
	        temp_var *= 1024; /*kilo*/
	      break;
	      case'm':
	      case'M':	      
		temp_var *= ( 1024 * 1024 );/*mega*/
	      break;
	      case'g':
	      case'G':	      
		temp_var *= ( 1024 * 1024 * 1024 );/*wow, giga *g */
	      break;
	      case'?':
		fprintf( stderr, "[Error] Wrong suffix behind -b, only b,k,m or g \n" );
	      exit(1); 
	      break;
	    }
	  /* 	  sscanf( optarg, "%lli", &temp_var ); */
	  seek_start = abs( temp_var / 2048 );
	  if( seek_start < 0 )
	    seek_start = 0;
          cut_flag = TRUE;
	  break;

	  case'c': /*chapter*/ /*NOT WORKING!!*/
	    if ( !isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Error] The thing behind -c has to be a number! \n" );
		exit(1); 
	      } 
	  sscanf( optarg, "%i", &chapid );
	  chapid--;/*in the ifo they start at zero */
	  break;

	  case'e': /*size to stop from the end (end-offset) */
	    if ( !isdigit( (int) optarg[0] ) )	     
	      { 
		fprintf( stderr, "[Error] The thing behind -e has to be a number! \n" );
		exit(1); 
	      } 
	  temp_var = atol( optarg );
	  size_suffix = strpbrk( optarg, "bkmgBKMG" );
	  if( !size_suffix)
	  {
	  	  fprintf( stderr, "[Error] Wrong suffix behind -b, only b,k,m or g \n" );
	      exit(1); 
	  }
	  switch( *size_suffix )
	    {
	      case'b':
   	      case'B':
		temp_var *= 512;  /*blocks (normal, not dvd)*/
	      break;
	      case'k':
   	      case'K':
		temp_var *= 1024; /*kilo*/
	      break;
	      case'm':
	      case'M':	      
		temp_var *= ( 1024 * 1024 );  /*mega*/
	      break;
	      case'g':
	      case'G':	      
		temp_var *= ( 1024 * 1024 * 1024 );/*wow, giga *g */
	      break;
	      case'?':
		fprintf( stderr, "[Error] Wrong suffix behind -b, only b,k,m or g \n" );
	      exit(1); 
	      break;
	    }

	  stop_before_end = abs( temp_var / 2048 );
	  if( stop_before_end < 0 )
	    stop_before_end = 0;
          cut_flag = TRUE;
	  break;

	  case'f': /*force flag, some options like -o, -1..-4 set this 
		     themselves */
	    force_flag = TRUE;
	  break;

	  case'h': /* good 'ol help */
	    usage( argv[0] );
	  break;

	  case'i': /*input dir, if the automatic needs to be overridden */
	    if ( isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Error] Erm, the number comes behind -n ... \n" );
		exit(1); 
	      } 
            fprintf( stderr, "[Hint] You use -i. Normally this is not necessary, vobcopy finds the input dir by itself. This option is only there if vobcopy makes trouble.\n" );
            fprintf( stderr, "[Hint] If vobcopy makes trouble, please mail me so that I can fix this (robos@muon.de). Thanks\n" );
            strncpy( provided_input_dir, optarg, 255 );
          if( strstr( provided_input_dir, "/dev" ) )
              {
                  fprintf( stderr, "[Error] Please don't use -i /dev/something in this version, only the next version will support this again.\n" );
                  fprintf( stderr, "[Hint] Please use the mount point instead (/cdrom, /dvd, /mnt/dvd or something)\n" );
              }
	  provided_input_dir_flag = TRUE;
	  break;

#if defined( HAS_LARGEFILE )
	  case'l': /*large file output*/
	   //max_filesize_in_blocks = 4500000000000000;
//	   max_filesize_in_blocks = 17179869184; //16 GB
	   max_filesize_in_blocks = 8388608; //16 GB /2048 (block)
	  /* 2^63 / 2048 (not exactly) */
	  large_file_flag = TRUE;
	  break;
#endif

 	  case'm':/*mirrors the dvd to harddrive completly*/
	    mirror_flag = TRUE; 
	  info_flag = TRUE;
	  break;
	  
	  case'n': /*title number*/
	    if ( !isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Error] The thing behind -n has to be a number! \n" );
		exit(1); 
	      } 
	  sscanf( optarg, "%i", &titleid );
	  titleid_flag = TRUE;
	  break;

	  case'o': /*output destination */
	    if ( isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Hint] Erm, the number comes behind -n ... \n" );
	      } 
            strncpy( provided_output_dir, optarg, 255 );
            if ( !strcasecmp( provided_output_dir, "stdout" ) || !strcasecmp( provided_output_dir, "-" ) )
                {
                    stdout_flag = TRUE;
                    force_flag = TRUE;
                }
            else
                {
                    provided_output_dir_flag = TRUE;
                    alternate_dir_count++;
                }
	  /* 	  force_flag = TRUE; */
	  break;

          case'q':/*quiet flag* - meaning no progress and such output*/
          quiet_flag = TRUE;
          break;

	  case'1': /*alternate output destination*/
	  case'2':
	  case'3':
	  case'4':
	  if( alternate_dir_count < options_char - 48 )
	      { 
		fprintf( stderr, "[Error] Please specify output dirs in this order: -o -1 -2 -3 -4 \n" );
		exit( 1 ); 
	      } 

	  if ( isdigit( (int) *optarg ) )
	    { 
	      fprintf( stderr, "[Hint] Erm, the number comes behind -n ... \n" );
	    } 
	  strncpy( alternate_output_dir[ options_char-49 ], optarg, 255 );
	  provided_output_dir_flag = TRUE;
	  alternate_dir_count++;
	  force_flag = TRUE;
	  break;

	  case't': /*provided title instead of the one from dvd, 
		     maybe even stdout output */
	    if ( strlen( optarg ) > 33 )
	      printf( "[Hint] The max title-name length is 33, the remainder got discarded" );
	  strncpy( provided_dvd_name, optarg, 33 );
	  provided_dvd_name_flag = TRUE;
	  if ( !strcasecmp( provided_dvd_name,"stdout" ) || !strcasecmp( provided_dvd_name,"-" ) )
	    {
	      stdout_flag = TRUE;
	      force_flag = TRUE;
	    }
	  break;
	  
	  case'v': /*verbosity level, can be called multiple times*/
	    strcpy( logfile_path, "/tmp/" ); 
	  verbose_flag = TRUE;
	  verbosity_level++;
	  break;
	  case'F': /*Fast-switch*/
	    if ( !isdigit( (int) *optarg ) )
	      { 
		fprintf( stderr, "[Error] The thing behind -F has to be a number! \n" );
		exit(1); 
	      } 
            sscanf( optarg, "%i", &fast_factor );
            if( fast_factor > BLOCK_COUNT ) //atm is BLOCK_COUNT == 64
                {
                    fprintf( stderr, "[Hint] The largest value for -F is %d at the moment - used that one...\n", BLOCK_COUNT );
                    fast_factor = BLOCK_COUNT;
                }

	    fast_switch = TRUE;
	  break;
	    
	  case'I': /*info, doesn't do anything, but simply prints some infos 
		     ( about the dvd )*/
	    info_flag = TRUE;
	  break;
	    
	  case'L': /*logfile-path (in case your system crashes every time you 
		     call vobcopy (and since the normal logs get written to 
		     /tmp and that gets cleared at every reboot... )*/
	    strncpy( logfile_path, optarg, 255 );	       
	  strcat( logfile_path, "/" );
	  verbose_flag = TRUE;
	  verbosity_level = 2;
	  break;

          case'O': /*only one file will get copied*/
          onefile_flag = TRUE;
          /*couldn't this be done like this: strncpy( onefile, optarg, sizeof( onefile ) - 1 ); and __shouldn't strncpy be made this way always! */
          strncpy( onefile, optarg, 254 );	       
          i = 0; /*this i here could be bad... */
          while( onefile[ i ] )
		{
		  onefile[ i ] = toupper ( onefile[ i ] );
		  i++;
		}
          if( onefile[ i - 1 ] == ',' )
              onefile[ i ] = 0;
          else
              {
                  onefile[ i ] = ',';
                  onefile[ i + 1 ] = 0;
              }
          mirror_flag = TRUE; 
          i = 0;
          break;
	  case'V': /*version number output */
	    printf( "Vobcopy "VERSION" - GPL Copyright (c) 2001 - 2006 robos@muon.de\n" );
	  exit( 0 );
	  break;
	    
	  case'?': /*probably never gets here, the others should catch it */
	    fprintf( stderr, "[Error] Wrong option.\n" );
	  usage( argv[0] );
	  exit( 1 );
	  break;

#ifndef HAVE_GETOPT_LONG
	  case'-': /* no getopt, complain */
	    fprintf( stderr, "[Error] %s was compiled without support for long options.\n",  argv[0] );
	  usage( argv[0] );
	  exit( 1 );
	  break;
#endif
	    
	default:  /*probably never gets here, the others should catch it */
	  fprintf( stderr, "[Error] Wrong option.\n" );
	  usage( argv[0] );
	  exit( 1 );
	}
    }

  fprintf( stderr, "Vobcopy "VERSION" - GPL Copyright (c) 2001 - 2006 robos@muon.de\n" );
  fprintf( stderr, "[Hint] All lines starting with \"libdvdread:\" are not from vobcopy but from the libdvdread-library\n" );
if( quiet_flag )
{
    fprintf( stderr, "[Hint] Quiet mode - All messages will now end up in /tmp/vobcopy.bla\n" );
    if ( freopen( "/tmp/vobcopy.bla" , "a" , stderr ) == NULL )
        { 
            printf( "[Error] Aaah! Re-direct of stderr to /tmp/vobcopy.bla didn't work! If -f is not used I stop here... \n" );
            printf( "[Hint] Use -f to continue (at your risk of stupid ascii text ending up in your vobs\n" );
            if ( !force_flag )
                exit( 1 );
        }
}


  if( verbosity_level > 1 ) /* this here starts writing the logfile */
    {
      fprintf( stderr, "[Info] Uhu, super-verbose\n" );
      strcpy( logfile_name, logfile_path );
      strcat( logfile_name, "vobcopy_" );
      strcat( logfile_name, VERSION );
      strcat( logfile_name, ".log" );
      fprintf( stderr, "[Info] The log-file is written to %s\n", logfile_name );
      fprintf( stderr, "[Hint] Make sure that vobcopy doesn't have to ask questions (like overwriting of old files), these end up in the log file..." );
      fprintf( stderr, "[Hint] If you don't like that position, use -L /path/to/logfile/ instead of -v -v\n" );
      if ( freopen( logfile_name, "a" , stderr ) == NULL )
        {
          printf( "[Error] Aaah! Re-direct of stderr to %s didn't work! \n", logfile_name );
	  /* oh no! redirecting of stderr failed, do best to quit gracefully */
	  exit( 1 );
	}

      strcpy( vobcopy_call, argv[0] );
      for( argc_i = 1; argc_i != argc; argc_i++ )
	{
	  strcat( vobcopy_call, " " );
	  strcat( vobcopy_call, argv[argc_i] );
	}
      fprintf( stderr, "--------------------------------------------------------------------------------\n" );
      fprintf( stderr, "[Info] Called: %s\n", vobcopy_call );
    }
  
  /*sanity check: -m and -n are mutually exclusive... */
  if( titleid_flag && mirror_flag )
    {
	  fprintf( stderr, "\n[Error] There can be only one: either -m or -n...'\n" );
	  exit( 1 );
    }


  /*
   * Check if the provided path is too long
   */
  if ( optind < argc ) /* there is still the path as in vobcopy-0.2.0 */
    {
      provided_input_dir_flag = TRUE;
      if ( strlen( argv[optind] ) >= 255 )
	{
	  fprintf( stderr, "\n[Error] Bloody path to long '%s'\n", argv[optind] );
	  exit( 1 );
	}
      strncpy( provided_input_dir, argv[optind],255 );
    }

  if ( provided_input_dir_flag ) /*the path has been given to us */
    {   
      int result;
      /* 'mounted' is an enum, it should not get assigned an int -- lb */
      if ( ( result = get_device( provided_input_dir, dvd_path ) ) < 0 )
	{
	  fprintf( stderr, "[Error] Could not get the device which belongs to the given path!\n" );
	  exit( 1 );
	}
      if (result == 0)
        mounted = FALSE;

      if (result == 1)
        mounted = TRUE;
    }
  else /*need to get the path and device ourselves ( oyo = on your own ) */
    {
      if ( ( dvd_count = get_device_on_your_own( provided_input_dir, dvd_path ) ) <= 0 )
	{
	  fprintf( stderr, "[Error] Could not get the device and path! Maybe not mounted the dvd?\n" );
	  exit( 1 );
	}
      if( dvd_count > 0 )
	mounted = TRUE;
    }

  /* 
   * Is the path correct
   */
  fprintf( stderr, "\n[Info] Path to dvd: %s\n", dvd_path );
   
  if( !( dvd = DVDOpen( dvd_path ) ) ) 
    {
      fprintf( stderr, "\n[Error] Path thingy didn't work '%s'\n", dvd_path);
      fprintf( stderr, "[Error] Try something like -i /cdrom, /dvd  or /mnt/dvd \n" );
      if( dvd_count > 1 ) 
	fprintf( stderr, "[Hint] By the way, you have %i cdroms|dvds mounted, that probably caused the problem\n", dvd_count );
      DVDClose( dvd );
      exit( 1 );
    }
   
  /*
   * this here gets the dvd name
   */
  if( !provided_dvd_name_flag ) 
      {
          get_dvd_name_return = get_dvd_name( dvd_path, dvd_name );
          fprintf( stderr, "[Info] Name of the dvd: %s\n", dvd_name );
      }
  else 
      {
          strncpy( dvd_name, provided_dvd_name, 33 );
      }

  /* The new part taken from play-title.c*/

  /**
   * Load the video manager to find out the information about the titles on
   * this disc.
   */
  vmg_file = ifoOpen( dvd, 0 );
  if( !vmg_file ) {
    fprintf( stderr, "[Error] Can't open VMG info.\n" );
    DVDClose( dvd );
    return -1;
  }
  tt_srpt = vmg_file->tt_srpt;

  /**
   * Get the title with the most chapters since this is probably the main part
   */
  for( i = 0;i < tt_srpt->nr_of_srpts;i++ )
    {
      sum_chapters += tt_srpt->title[ i ].nr_of_ptts;
      if( i > 0 )
	{
	  if( tt_srpt->title[ i ].nr_of_ptts > tt_srpt->title[ most_chapters ].nr_of_ptts )
	    {
	      most_chapters = i;
	    }
	}
      
    }


  if( !titleid_flag ) /*no title specified (-n ) */
    {
      titleid = most_chapters + 1; /*they start counting with 0, I with 1...*/
    }
  

  /**
   * Make sure our title number is valid.
   */
  fprintf( stderr, "[Info] There are %d titles on this DVD.\n",
	   tt_srpt->nr_of_srpts );
  if( titleid <= 0 || ( titleid-1 ) >= tt_srpt->nr_of_srpts ) {
    fprintf( stderr, "[Error] Invalid title %d.\n", titleid );
    ifoClose( vmg_file );
    DVDClose( dvd );
    return -1;
  }


  /**
   * Make sure the chapter number is valid for this title.
   */

  fprintf( stderr, "[Info] There are %i chapters on the dvd.\n", sum_chapters );
  fprintf( stderr, "[Info] Most chapters has title %i with %d chapters.\n", 
	   ( most_chapters + 1 ), tt_srpt->title[ most_chapters ].nr_of_ptts );

  if( info_flag )
    {
      fprintf( stderr, "[Info] All titles:\n" );
      for( i = 0;i < tt_srpt->nr_of_srpts;i++ )
	{
	  int chapters = tt_srpt->title[ i ].nr_of_ptts;
	  if( chapters > 1 )
	    fprintf( stderr, "[Info] Title %i has %d chapters.\n", 
		     ( i+1 ), chapters );
	  else
	    fprintf( stderr, "[Info] Title %i has %d chapter.\n", 
		     ( i+1 ), chapters );

	}
    }
  


  if( chapid < 0 || chapid >= tt_srpt->title[ titleid-1 ].nr_of_ptts ) {
    fprintf( stderr, "[Error] Invalid chapter %d\n", chapid + 1 );
    ifoClose( vmg_file );
    DVDClose( dvd );
    return -1;
  }

  /**
   * Make sure the angle number is valid for this title.
   */
  for( i = 0;i < tt_srpt->nr_of_srpts;i++ )
    {
      sum_angles += tt_srpt->title[ i ].nr_of_angles;
    }

  fprintf( stderr, "\n[Info] There are %d angles on this dvd.\n", sum_angles );
  if( angle < 0 || angle >= tt_srpt->title[ titleid-1 ].nr_of_angles ) {
    fprintf( stderr, "[Error] Invalid angle %d\n", angle + 1 );
    ifoClose( vmg_file );
    DVDClose( dvd );
    return -1;
  }

  if( info_flag )
    {
      fprintf( stderr, "[Info] All titles:\n" );
      for(i = 0;i < tt_srpt->nr_of_srpts;i++ )
	{
	  int angles = tt_srpt->title[ i ].nr_of_angles;
	  if( angles > 1 )
	    fprintf( stderr, "[Info] Title %i has %d angles.\n", 
		     ( i+1 ), angles );
	  else
	    fprintf( stderr, "[Info] Title %i has %d angle.\n", 
		     ( i+1 ), angles );

	}

      disk_vob_size = get_used_space( provided_input_dir, verbosity_level );
    }

   
  /*
   * get the whole vob size via stat( ) manually
   */
//  if( mounted ) 
  if( mounted && !mirror_flag ) 
    {
      vob_size = ( get_vob_size( titleid, provided_input_dir ) ) - 
	( seek_start * 2048 ) - ( stop_before_end * 2048 );
	if( vob_size == 0 || vob_size > 9663676416) /*9663676416 equals 9GB */
	{
		fprintf( stderr, "\n[Error] Something went wrong during the size detection of the" );
		fprintf( stderr, "\n[Error] vobs, size check at the end won't work (probably), but I continue anyway\n\n" );
		vob_size = 0;
	}
    } 
  
  if( mirror_flag ) /* this gets the size of the whole dvd */
    {
      add_end_slash( provided_input_dir );
      disk_vob_size = get_used_space( provided_input_dir, verbosity_level );
/*       vob_size = get_free_space( provided_input_dir,verbosity_level ); */
    }
  

  /* 
   * check if on the target directory is enough space free
   * see man statfs for more info
   */
  
  /*get the current working directory*/
  if ( provided_output_dir_flag )
    {
      strcpy( pwd, provided_output_dir );
    }
  else
    {
      if ( getcwd( pwd, 255 ) == NULL )
	{ 
	  fprintf( stderr, "\n[Error] Hmm, the path length of your current directory is really large (>255)\n" );
	  fprintf( stderr, "[Hint] Change to a path with shorter path length pleeeease ;-)\n" );
	  exit( 1 );   
	}
    }
  
  add_end_slash( pwd );
  
  pwd_free = get_free_space( pwd,verbosity_level );

  if( fast_switch )
      block_count = fast_factor;
  else
      block_count = 1;

  /***
    this is the mirror section
  ***/

  if( mirror_flag ) 
    {/*mirror beginning*/
      fprintf( stderr,"\n[Info] DVD-name: %s\n", dvd_name );
      fprintf( stderr, "[Info]  Disk free: %.0f MB\n", (float) pwd_free / ( 1024*1024 ) );
      fprintf( stderr, "[Info]  Vobs size: %.0f MB\n", (float) disk_vob_size / ( 1024*1024 ) ); 
      if( ( force_flag || pwd_free > disk_vob_size ) && alternate_dir_count < 2 )
	/* no dirs behind -1, -2 ... since its all in one dir */
	{
	  char video_ts_dir[263];
	  char number[8], nr[4];
	  char input_file[280];
	  char output_file[255];
	  int  i, start, title_nr = 0;
	  off_t file_size;
	  double percent = 0, tmp_i = 0, tmp_file_size = 0;
	  int k = 0;
	  char d_name[256];
		      
	  strncpy( name, pwd, 255 );
 	  strncat( name, dvd_name, 33 ); 

          if( !stdout_flag )
              {
                  makedir ( name );
                  
                  strcat( name, "/VIDEO_TS/" ); 
                  
                  makedir ( name );
                  
                  fprintf( stderr,"[Info] Writing files to this dir: %s\n", name );
              }
//TODO: substitute with open_dir function
	  strcpy( video_ts_dir, provided_input_dir );
	  strcat( video_ts_dir, "video_ts"); /*it's either video_ts */
	  dir = opendir( video_ts_dir );     /*or VIDEO_TS*/
	  if ( dir == NULL )
	    {
	      strcpy( video_ts_dir, provided_input_dir );
	      strcat( video_ts_dir, "VIDEO_TS");
	      dir = opendir( video_ts_dir );
	      if ( dir == NULL )
		{
		  fprintf( stderr, "[Error] Hmm, weird, the dir video_ts|VIDEO_TS on the dvd couldn't be opened\n");
		  fprintf( stderr, "[Error] The dir to be opened was: %s\n", video_ts_dir );
		  fprintf( stderr, "[Hint] Please mail me what your vobcopy call plus -v -v spits out\n");
		  exit( 1 );
		}
	    }

	  directory = readdir( dir ); /* thats the . entry */
	  directory = readdir( dir ); /* thats the .. entry */
	  /* according to the file type (vob, ifo, bup) the file gets copied */
	  while( ( directory = readdir( dir ) ) != NULL )
	    {/*main mirror loop*/

	      k = 0;
	      strncpy( output_file, name, 255 );
	      /*in dvd specs it says it must be uppercase VIDEO_TS/VTS...
		but iso9660 mounted dvd's sometimes have it lowercase */
	      while( directory->d_name[k] )
		{
		  d_name[k] = toupper (directory->d_name[k] );
		  k++;
		}
              d_name[k] = 0;

                /*in order to copy only _one_ file */
/*                if( onefile_flag )
                {
                    if( !strstr( onefile, d_name ) ) maybe other way around*/
/*                    continue; 
                }*/
              /*in order to copy only _one_ file and do "globbing" a la: -O 02 will copy vts_02_1, vts_02_2 ... all that have 02 in it*/
              if( onefile_flag )
                  {
                      char *tokenpos, *tokenpos1;
                      char tmp[12];
                      tokenpos = onefile;
                      if( strstr( tokenpos, "," ) )
                          {          
                              while( ( tokenpos1 = strstr( tokenpos, "," ) ) ) /*tokens separated by , */
                                  { /*this parses every token without modifying the onefile var */
                                      int len_begin;
                                      len_begin = tokenpos1 - tokenpos;
/*                                      printf(" debug: len = %d tokenpos = %d tokenpos1 = %d\n", len_begin, tokenpos, tokenpos1 ); */
                                      strncpy( tmp, tokenpos, len_begin );
                                      tokenpos = tokenpos1 + 1;
/*                                      printf(" debug: tokenpos = %d tokenpos1 = %d\n", tokenpos, tokenpos1 ); */
                                      tmp[ len_begin ] = '\0';
/*                                      printf(" debug: token = %s\n", tmp ); */
                                      if( strstr( d_name, tmp ) ) 
                                          goto next; /*if the token is found in the d_name copy */
                                      else
                                          continue; /* next token is tested */
                                  }
                              continue; /*no token matched, next d_name is tested */
                          }
                      else
                          {
/*                              printf(" debug: onefile = %s\n", onefile ); */
                              if( !strstr( d_name, onefile ) ) /*maybe other way around */
                                  {
                                      continue; 
                                  }
                              else
                                  goto next; /*if the token is found in the d_name copy */
                              continue; /*no token matched, next d_name is tested */
                          }
                  }
          next: /*for the goto - ugly, I know... */

      if( stdout_flag ) /*this writes to stdout*/
          {
              streamout = STDOUT_FILENO; /*in other words: 1, see "man stdout" */
          }
      else
          {
	     if( strstr( d_name, ";?" ) )
	       {
		  fprintf( stderr, "\n[Hint] File on dvd ends in \";?\" (%s)\n", d_name );
		  strncat( output_file, d_name, strlen( d_name ) - 2 );
	       }
	     else
	       {
		  strcat( output_file, d_name );
	       }
	     
	      fprintf(stderr, "[Info] \nWriting to %s \n", output_file);

	      if( open( output_file, O_RDONLY ) >= 0 )
		{
                    bool bSkip = FALSE;
                    fprintf( stderr,"\n[Error] File '%s' already exists, [o]verwrite, [s]kip or [q]uit? \n", output_file );
                    /*TODO: add [a]ppend  and seek thought stream till point of append is there */
		  while ( 1 )
		    {
//		      op=fgetc( stdin );
                      while ((op = fgetc (stdin)) == EOF)
                          usleep (1);
		      fgetc ( stdin ); /* probably need to do this for second 
					  time it comes around this loop */
		      if( op == 'o' )
			{
			  if( ( streamout = open( output_file, O_WRONLY | O_TRUNC ) ) < 0 )
			    {
			      fprintf( stderr, "\n[Error] Error opening file %s\n", output_file );
                              fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
			      exit ( 1 );
			    }
			  else
			    close (streamout);
			  overwrite_flag = TRUE;
			  break;
			}
		      else if( op == 'q' )
			{
			  DVDCloseFile( dvd_file );
			  DVDClose( dvd );
			  exit( 1 );
			}
                      else if( op == 's' )
                          {
                              bSkip = TRUE;
                              break;
                          }
                      else
			{
			  fprintf( stderr, "\n[Hint] Please choose [o]verwrite or [q]uit the next time ;-)\n" );
			}
		    }
                  if( bSkip )
                   continue; /* next file, please! */
		}

	      strcat( output_file, ".partial" );

	      if( open( output_file, O_RDONLY ) >= 0 )
		{
                    fprintf( stderr,"\n[Error] File '%s' already exists, [o]verwrite or [q]uit? \n", output_file );
                    /*TODO: add [a]ppend  and seek thought stream till point of append is there */
		  while ( 1 )
		    {
//		      op=fgetc( stdin );
                      while ((op = fgetc (stdin)) == EOF)
                          usleep (1);
		      fgetc ( stdin ); /* probably need to do this for second 
					  time it comes around this loop */
		      if( op == 'o' )
			{
			  if( ( streamout = open( output_file, O_WRONLY | O_TRUNC ) ) < 0 )
			    {
			      fprintf( stderr, "\n[Error] Error opening file %s\n", output_file );
                              fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
			      exit ( 1 );
			    }
			  else
			    close( streamout );
			  overwrite_flag = TRUE;
			  break;
			}
		      else if( op == 'q' )
			{
			  DVDCloseFile( dvd_file );
			  DVDClose( dvd );
			  exit( 1 );
			}
		      else
			{
			  fprintf( stderr, "\n[Hint] Please choose [o]verwrite or [q]uit the next time ;-)\n" );
			}
		    }
		}
	      else
		{
		  /*assign the stream */
		  if( ( streamout = open( output_file, O_WRONLY | O_CREAT, 0644 ) ) < 0 )
		    {
		      fprintf( stderr, "\n[Error] Error opening file %s\n", output_file );
              fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
		      exit ( 1 );
		    }
		}
          }    
              /* get the size of that file*/
	      strcpy( input_file, video_ts_dir );
	      strcat( input_file, "/" );
	      strcat( input_file, directory->d_name );
	      stat( input_file, &buf );
	      file_size = buf.st_size;
	      tmp_file_size = file_size;

	      memset( bufferin, 0, DVD_VIDEO_LB_LEN * sizeof( unsigned char ) );	      

	      /*this here gets the title number*/
	      for( i = 1; i <= 99; i++ ) /*there are 100 titles, but 0 is  
					   named video_ts, the others are 
					   vts_number_0.bup */ 
		{ 
                    sprintf(number, "_%.2i", i);
		  
		  if ( strstr( directory->d_name, number ) ) 
		    {
		      title_nr = i;

		      break; /*number found, is in i now*/
		    }
		  /*no number -> video_ts is the name -> title_nr = 0*/
		}
	      
	      /*which file type is it*/
	      if( strstr( directory->d_name, ".bup" ) 
		   || strstr( directory->d_name, ".BUP" ) )
		{
		  dvd_file = DVDOpenFile( dvd, title_nr, DVD_READ_INFO_BACKUP_FILE );
		  /*this copies the data to the new file*/
		  for( i = 0; i*DVD_VIDEO_LB_LEN < file_size; i++)
		    {
		      DVDReadBytes( dvd_file, bufferin, DVD_VIDEO_LB_LEN );
		      if( write( streamout, bufferin, DVD_VIDEO_LB_LEN ) < 0 )
			{
			  fprintf( stderr, "\n[Error] Error writing to %s \n", output_file );
			  fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
			  exit( 1 );
			}
		      /* progress indicator */
		      tmp_i = i;
		      fprintf( stderr, "%4.0fkB of %4.0fkB written\r", 
			      ( tmp_i+1 )*( DVD_VIDEO_LB_LEN/1024 ), tmp_file_size/1024 );
		    }
		  fprintf( stderr, "\n");
                  if( !stdout_flag )
                      {
                         if( fdatasync( streamout ) < 0 )
                           {
                             fprintf( stderr, "\n[Error] error writing to %s \n", output_file );
                             fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
                             exit( 1 );
                           }

                          close( streamout );
                          re_name( output_file );
                      }
		}
	      
	      if( strstr( directory->d_name, ".ifo" )
		   || strstr( directory->d_name, ".IFO" ) )
		{
		  dvd_file = DVDOpenFile( dvd, title_nr, DVD_READ_INFO_FILE );

		  /*this copies the data to the new file*/
		  for( i = 0; i*DVD_VIDEO_LB_LEN < file_size; i++)
		    {
		      DVDReadBytes( dvd_file, bufferin, DVD_VIDEO_LB_LEN );
		      if( write( streamout, bufferin, DVD_VIDEO_LB_LEN ) < 0 )
			{
			  fprintf( stderr, "\n[Error] Error writing to %s \n", output_file );
			  fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
			  exit( 1 );
			}
		      /* progress indicator */
		      tmp_i = i;
		      fprintf( stderr, "%4.0fkB of %4.0fkB written\r", 
			      ( tmp_i+1 )*( DVD_VIDEO_LB_LEN/1024 ), tmp_file_size/1024 );
		    }
		  fprintf( stderr, "\n");
                  if( !stdout_flag )
                      {
                         if( fdatasync( streamout ) < 0 )
                           {
                             fprintf( stderr, "\n[Error] error writing to %s \n", output_file );
                             fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
                             exit( 1 );
                           }

                          close( streamout );
                          re_name( output_file );
                      }
		}

	      if( strstr( directory->d_name, ".vob" )
		   || strstr( directory->d_name, ".VOB"  ) )
		{
                    int l=0;
 		  if( directory->d_name[7] == 48 || title_nr == 0  )  
		  {
		    /*this is vts_xx_0.vob or video_ts.vob, a menu vob*/
		    dvd_file = DVDOpenFile( dvd, title_nr, DVD_READ_MENU_VOBS );
		    start = 0 ;
		  }
		  else
		    {
		    dvd_file = DVDOpenFile( dvd, title_nr, DVD_READ_TITLE_VOBS );
		    }
		  if( directory->d_name[7] == 49 || directory->d_name[7] == 48 ) /* 49 means in ascii 1 and 48 0 */ 
                      {
                          /* reset start when at beginning of Title */
                          start = 0 ;
                      }
                  if( directory->d_name[7] > 49 && directory->d_name[7] < 58 ) /* 49 means in ascii 1 and 58 :  (i.e. over 9)*/ 
                      {
			 off_t culm_single_vob_size = 0;
			 int a, subvob; 
//			 printf( "debug: title = %d \n", title_nr );
			 
			 subvob = ( directory->d_name[7] - 48 );
//			 printf( "debug: subvob = %d \n", subvob );
			 
			 for( a = 1; a < subvob; a++ )
			   {
                               if( strstr( input_file, ";?" ) )
                              input_file[ strlen( input_file ) - 7 ] = ( a + 48 );
                               else
                                input_file[ strlen( input_file ) - 5 ] = ( a + 48 );

/*			      input_file[ strlen( input_file ) - 5 ] = ( a + 48 );*/
			      if( stat( input_file, &buf ) < 0 )
				{
				   fprintf( stderr, "[Info] Can't stat() %s.\n", input_file );
				   exit( 1 );
				}

			      culm_single_vob_size += buf.st_size;
			      if( verbosity_level > 1 )
				fprintf( stderr, "[Info] Vob %d %d (%s) has a size of %lli\n", title_nr, subvob, input_file, buf.st_size );
			   }
			 			 
			 start = ( culm_single_vob_size / DVD_VIDEO_LB_LEN ); /* this here seeks d_name[7] 
//                          start = ( ( ( directory->d_name[7] - 49 ) * 512 * 1024 ) - ( directory->d_name[7] - 49 ) ); /* this here seeks d_name[7] 
                              (which is the 3 in vts_01_3.vob) Gigabyte (which is equivalent to 512 * 1024 blocks 
                              (a block is 2kb) in the dvd stream in order to reach the 3 in the above example.
			 * NOT! the sizes of the "1GB" files aren't 1GB... */
                      }

                              /*this copies the data to the new file*/
		   if( verbosity_level > 1) 
		     fprintf( stderr, "[Info] Start of %s at %d blocks \n", output_file, start );
                   file_block_count = block_count;
                   for( i = start; ( i - start ) * DVD_VIDEO_LB_LEN < file_size; i += file_block_count)
		    {
                     /* Only read and write as many blocks as there are left in the file */
                     if ( ( i - start + file_block_count ) * DVD_VIDEO_LB_LEN > file_size )
                       {
                         file_block_count = ( file_size / DVD_VIDEO_LB_LEN ) - ( i - start );
                       }

/*		      DVDReadBlocks( dvd_file, i, 1, bufferin );this has to be wrong with the 1 there...*/
       		      blocks = DVDReadBlocks( dvd_file, i, file_block_count, bufferin );

		      if( write( streamout, bufferin, DVD_VIDEO_LB_LEN * blocks ) < 0 )
			{
			  fprintf( stderr, "\n[Error] Error writing to %s \n", output_file );
			  fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
			  exit( 1 );
			}

                      l += blocks; 
		      /*progression bar*/
                      /*this here doesn't work with -F 10 */
/*		      if( !( ( ( ( i-start )+1 )*DVD_VIDEO_LB_LEN )%( 1024*1024 ) ) ) */
                      if( l > 100 )
			{
			  tmp_i = ( i-start );

  			  percent = ( ( ( ( tmp_i+1 )*DVD_VIDEO_LB_LEN )*100 )/tmp_file_size );  
			  fprintf( stderr, "%4.0fMB of %4.0fMB written ",
				  ( ( tmp_i+1 )*DVD_VIDEO_LB_LEN )/( 1024*1024 ),
				  ( tmp_file_size+2048 )/( 1024*1024 ) );
 			  fprintf( stderr,"( %3.1f %% ) \r", percent ); 
                          l = 0;
			}
		      
		    }
		  start=i;
		  fprintf( stderr, "\n" );
                  if( !stdout_flag )
                      {
                         if( fdatasync( streamout ) < 0 )
                           {
                             fprintf( stderr, "\n[Error] error writing to %s \n", output_file );
                             fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
                             exit( 1 );
                           }

                          close( streamout );
                          re_name( output_file );
                      }
		}
	    }

	  ifoClose( vmg_file );
	  DVDCloseFile( dvd_file );
	  DVDClose( dvd );
	  exit( 0 );
	}
      else
	{
	  fprintf( stderr, "[Error] Not enough free space on the destination dir. Please choose another one or -f\n" );
	  fprintf( stderr, "[Error] or dirs behind -1, -2 ... are NOT allowed with -m!\n" );
          exit( 1 );
	}
    }
                         /*end of mirror block*/



  
  
  /*
   * Open now up the actual files for reading
   * they come from libdvdread merged together under the given title number
   * (thx again for the great library)
   */
  fprintf( stderr,"[Info] Using Title: %i\n", titleid );
  fprintf( stderr,"[Info] Title has %d chapters and %d angles\n",tt_srpt->title[ titleid - 1 ].nr_of_ptts,tt_srpt->title[ titleid - 1 ].nr_of_angles );
  fprintf( stderr,"[Info] Using Chapter: %i\n", chapid + 1 );
  fprintf( stderr,"[Info] Using Angle: %i\n", angle + 1 );

  
  if( info_flag && vob_size != 0 )
    {
      fprintf( stderr,"\n[Info] DVD-name: %s\n", dvd_name );
      fprintf( stderr, "[Info]  Disk free: %f MB\n", (double)  (pwd_free / ( 1024.0*1024.0 )) );
      fprintf( stderr, "[Info]  Vobs size: %f MB\n", (double)  vob_size / ( 1024.0*1024.0 ) ); 
      ifoClose( vmg_file );
      DVDCloseFile( dvd_file );
      DVDClose( dvd );
      /*hope all are closed now...*/
      exit( 0 );
    }
  

  /**
   * Load the VTS information for the title set our title is in.
   */
  vts_file = ifoOpen( dvd, tt_srpt->title[ titleid-1 ].title_set_nr );
  if( !vts_file ) {
    fprintf( stderr, "[Error] Can't open the title %d info file.\n",
	     tt_srpt->title[ titleid-1 ].title_set_nr );
    ifoClose( vmg_file );
    DVDClose( dvd );
    return -1;
  }


  /**
   * Determine which program chain we want to watch.  This is based on the
   * chapter number.
   */
  ttn = tt_srpt->title[ titleid-1 ].vts_ttn;
  vts_ptt_srpt = vts_file->vts_ptt_srpt;
  pgc_id = vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ].pgcn;
  pgn = vts_ptt_srpt->title[ ttn - 1 ].ptt[ chapid ].pgn;
  cur_pgc = vts_file->vts_pgcit->pgci_srp[ pgc_id - 1 ].pgc;
  start_cell = cur_pgc->program_map[ pgn - 1 ] - 1;


  /**
   * We've got enough info, time to open the title set data.
   */
  dvd_file = DVDOpenFile( dvd, tt_srpt->title[ titleid-1 ].title_set_nr,
			  DVD_READ_TITLE_VOBS );
  if( !dvd_file ) {
    fprintf( stderr, "[Error] Can't open title VOBS (VTS_%02d_1.VOB).\n",
	     tt_srpt->title[ titleid-1 ].title_set_nr );
    ifoClose( vts_file );
    ifoClose( vmg_file );
    DVDClose( dvd );
    return -1;
  }



  file_size_in_blocks = DVDFileSize( dvd_file );

  if ( vob_size == ( - ( seek_start * 2048 ) - ( stop_before_end * 2048 ) ) )
    {
      vob_size = ( ( off_t ) ( file_size_in_blocks ) * ( off_t ) DVD_VIDEO_LB_LEN ) - 
	( seek_start * 2048 ) - ( stop_before_end * 2048 );
      if( verbosity_level >= 1 )
	fprintf( stderr, "[Info] Vob_size was 0\n" );
    }
  

  /*debug-output: difference between vobsize read from cd and size returned from libdvdread */
  if ( mounted && verbose_flag )
    {
        fprintf( stderr,"\n[Info] Difference between vobsize read from cd and size returned from libdvdread:\n" );
/*        fprintf( stderr,"vob_size (stat) = %lu\nlibdvdsize      = %lu\ndiff            = %lu\n",  //TODO:the diff returns only crap...
                vob_size, 
                ( off_t ) ( file_size_in_blocks ) * ( off_t ) DVD_VIDEO_LB_LEN, 
                ( off_t ) vob_size - ( off_t ) ( ( off_t )( file_size_in_blocks ) * ( off_t ) ( DVD_VIDEO_LB_LEN ) ) ); */
        fprintf( stderr,"[Info] Vob_size (stat) = %lu\n[Info] libdvdsize      = %lu\n", 
                vob_size, 
                ( off_t ) ( file_size_in_blocks ) * ( off_t ) DVD_VIDEO_LB_LEN );
    }

  if( info_flag )
    {
      fprintf( stderr,"\n[Info] DVD-name: %s\n", dvd_name );
      fprintf( stderr, "[Info]  Disk free: %.0f MB\n", ( float ) (pwd_free / (1024 * 1024)) );
      /* Should be the *disk* size here, right? -- lb */
      fprintf( stderr, "[Info]  Vobs size: %.0f MB\n", ( float ) (disk_vob_size / (1024 * 1024 )) ); 

      ifoClose( vts_file );
      ifoClose( vmg_file );
      DVDCloseFile( dvd_file );
      DVDClose( dvd );
      /*hope all are closed now...*/
      exit( 0 );
    }

  
  /* now the actual check if enough space is free*/
  if ( pwd_free < vob_size )
    {
      fprintf( stderr, "\n[Info]  Disk free: %.0f MB", (float) pwd_free / ( 1024*1024 ) );
      fprintf( stderr, "\n[Info]  Vobs size: %.0f MB", (float) vob_size / ( 1024*1024 ) ); 
      if( !force_flag )
	fprintf( stderr, "\n[Error] Hmm, better change to a dir with enough space left or call with -f (force) \n" );
      if( pwd_free == 0 && !force_flag )
	{
	  if( verbosity_level > 1 )
	    fprintf( stderr,"[Error] Hmm, statfs (statvfs) seems not to work on that directory. \n" );
	  fprintf( stderr, "[Error] Hmm, statfs (statvfs) seems not to work on that directory. \n" );
          fprintf( stderr, "[Hint] Nevertheless, do you want vobcopy to continue [y] or do you want to check for \n" );
	  fprintf( stderr, "[Hint] enough space first [q]?\n" );
	  
	  while ( 1 )
	    {
//	      op=fgetc( stdin );
              while ((op = fgetc (stdin)) == EOF)
                    usleep (1);
	      if( op == 'y' )
		{
		  force_flag = TRUE;
		  if( verbosity_level >= 1 )
		    fprintf( stderr, "[Info] y pressed - force write\n" );
		  break;
		}
	      else if( op == 'n' || op =='q' )
		{
		  if( verbosity_level >= 1 )
		    fprintf( stderr, "[Info] n/q pressed\n" );
		  exit( 1 );
		  break;
		}
	      else
		{
		  fprintf( stderr, "[Error] Please choose [y] to continue or [n] to quit\n" );
		}
	    }
	}
      if ( !force_flag )	  
	exit( 1 );
    }

 /*
from alt.tasteless.jokes
A golfer is in a competitive match with a friend, who is ahead by a couple
of strokes. "Boy, I'd give anything to sink this putt," the golfer mumbles
to himself. Just then, a stranger walks up beside him and whispers,"Would
you be willing to give up one-fourth of your sex life?"
Thinking the man is crazy and his answer will be meaningless, the golfer
also feels that maybe this is a good omen so he says, "Sure," and sinks the
putt.
Two holes later, he mumbles to himself again, "Gee, I sure would like to
get an eagle on this one." The same stranger is at his side again and
whispers, "Would it be ! worth giving up another fourth of your sex
ife?"
Shrugging, the golfer replies, "Okay." And he makes an eagle.
On the final hole, the golfer needs another eagle to win. Without waiting
for him to say anything, the stranger quickly moves to his side and
says,Would winning this match be worth giving up the rest of your sex life?"
Definitely," the golfer replies, and he makes the eagle.
As the golfer is walking to the club house, the stranger walks alongside
him and says, "I haven't really been fair with you because you don't know
who I am. I'm the devil, and from this day forward you will have no sex
life."
"Nice to meet you," the golfer replies, "I'm Father O'Malley." 
*/

  /********************* 
   * this is the main read and copy loop
   *********************/
  fprintf( stderr,"\n[Info] DVD-name: %s\n", dvd_name );
  if( provided_dvd_name_flag && !stdout_flag ) 
    /*if the user has given a name for the file */
    {
      fprintf( stderr, "\n[Info] Your-name for the dvd: %s\n", provided_dvd_name );
      strcpy( dvd_name, provided_dvd_name );
    }
  
  while( offset < ( file_size_in_blocks - seek_start - stop_before_end ) )
    {
      partcount++; 

      if( !stdout_flag ) /*if the stream doesn't get written to stdout*/
	{
	  /*part to distribute the files over different directories*/
	  if( paths_taken == 0 )
	    {
	      add_end_slash( pwd );
	      free_space = get_free_space( pwd, verbosity_level );
	      if( verbosity_level > 1 )
		fprintf( stderr,"[Info] Free space for -o dir: %.0f\n", ( float ) free_space );
              if( large_file_flag )
                  make_output_path( pwd,name,get_dvd_name_return,dvd_name,titleid, -1 );
              else
                  make_output_path( pwd,name,get_dvd_name_return,dvd_name,titleid, partcount );
	    }
	  else
	    {
/* 	      if( alternate_dir_count == 1 && !space_greater_2gb_flag )  */
		/*shouldn't come to this only if some data is still there but
		  no free space left... */
/* 		{ */
/* 		  fprintf( stderr,  */
/* 			   "\nHmm, looks like there wasn't enough space in the provided dirs ...\n" */
/* 			   "Did you use -1, -2, -3 or even up to -4 to specify alternate dirs?\n" ); */
/* 		  exit( 1 ); */
/* 		} */
/* 	      for( i = 1; i < 5; i++ ) */
/* 		{ */
/* 		  if( paths_taken == i && alternate_dir_count > 1 ) */

	      for( i = 1; i < alternate_dir_count; i++ )
		{
		  if( paths_taken == i )
		    {
		      add_end_slash( alternate_output_dir[ i-1 ] );
		      free_space = get_free_space( alternate_output_dir[ i-1 ],verbosity_level );

		      if( verbosity_level > 1 )
			fprintf( stderr,"[Info] Free space for -%i dir: %.0f\n", i, ( float ) free_space );
		      if ( large_file_flag )
                          make_output_path( alternate_output_dir[ i-1 ], name, get_dvd_name_return, dvd_name, titleid, -1 );
                      else
                          make_output_path( alternate_output_dir[ i-1 ], name, get_dvd_name_return, dvd_name, titleid,partcount );
/* 			alternate_dir_count--; */
		    }
		}
	    }
	  /*here the output size gets adjusted to the given free space*/
	      
	  if( !large_file_flag && force_flag && free_space < 2147473408 ) /* 2GB */
	    {
	      space_greater_2gb_flag = FALSE;
	      max_filesize_in_blocks = ( ( free_space - 2097152 ) / 2048 ); /* - 2 MB */
	      if( verbosity_level > 1 )
		fprintf( stderr, "[Info] Taken max_filesize_in_blocks(2GB version): %.0f\n", ( float ) max_filesize_in_blocks );
	      paths_taken++;
	    }
	  else if( large_file_flag && force_flag) /*lfs version */
	    {
	      space_greater_2gb_flag = FALSE;
	      max_filesize_in_blocks = ( ( free_space - 2097152) / 2048);/* - 2 MB */
	      if( verbosity_level > 1)
		fprintf( stderr,"[Info] Taken max_filesize_in_blocks(lfs version): %.0f\n", ( float ) max_filesize_in_blocks );
	      paths_taken++;
	    }
	  else if( !large_file_flag )
	    {
	      max_filesize_in_blocks = 1048571; /*if free_space is more than  2 GB fall back to max_filesize_in_blocks=2GB*/
	      space_greater_2gb_flag = TRUE;
	    }


	      if( open( name, O_RDONLY ) >= 0 )
		{
                    fprintf( stderr,"\n[Error] File '%s' already exists, [o]verwrite or [q]uit? \n", name );
                    /*TODO: add [a]ppend  and seek thought stream till point of append is there */
		  while ( 1 )
		    {
//		      op=fgetc( stdin );
                      while ((op = fgetc (stdin)) == EOF)
                          usleep (1);
		      fgetc ( stdin ); /* probably need to do this for second time it comes around this loop */
		      if( op == 'o' )
			{
			  if( ( streamout = open( name, O_WRONLY | O_TRUNC ) ) < 0 )
			    {
			      fprintf( stderr, "\n[Error] Error opening file %s\n", name );
			      fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
			      exit ( 1 );
			    }
			  else
			    close( streamout );
			  overwrite_flag = TRUE;
			  break;
			}
		      else if( op == 'q' )
			{
			  DVDCloseFile( dvd_file );
			  DVDClose( dvd );
			  exit( 1 );
			}
		      else
			{
			  fprintf( stderr, "\n[Hint] please choose [o]verwrite or [q]uit the next time ;-)\n" );
			}
		    }
		}
          
          strcat( name, ".partial" );
          
#if defined( HAS_LARGEFILE )
	  if( open( name, O_RDONLY|O_LARGEFILE ) >= 0 )
#else
	    if( open( name, O_RDONLY ) >= 0 )
#endif
	      {
		if ( get_free_space( name, verbosity_level ) < 2097152 ) 
		  /* it might come here when the platter is full after a -f */
		  {
		    fprintf( stderr, "[Error] Seems your platter is full...\n");
		    exit ( 1 );
		  }
		fprintf( stderr, "\n[Error] File '%s' already exists, [o]verwrite, [a]ppend, [q]uit? \n", name );
		while ( 1 )
		  {
//		    op=fgetc( stdin );
                    while ((op = fgetc (stdin)) == EOF)
                         usleep (1);
		    fgetc ( stdin ); /* probably need to do this for second time it 
				       comes around this loop */
		    if( op == 'o' )
		      {
#if defined( HAS_LARGEFILE )
			if( ( streamout = open( name, O_WRONLY | O_TRUNC | O_LARGEFILE ) ) < 0 )
#else
			  if( ( streamout = open( name, O_WRONLY | O_TRUNC ) ) < 0 )
#endif
			    {
			      fprintf( stderr, "\n[Error] Error opening file %s\n", name );
			      exit ( 1 );
			    }
			  else
			    close (streamout);
			overwrite_flag = TRUE;
			break;
		      }
		    else if( op == 'a' )
		      {
#if defined( HAS_LARGEFILE )
			if( ( streamout = open( name, O_WRONLY | O_APPEND | O_LARGEFILE ) ) < 0 )
#else
			  if( ( streamout = open( name, O_WRONLY | O_APPEND ) ) < 0 )
#endif
			    {
			      fprintf( stderr, "\n[Error] Error opening file %s\n", name );
			      exit ( 1 );
			    }
			  else
			    close( streamout );
			if( verbosity_level >= 1 )
			  fprintf( stderr, "[Info] User chose append\n" );
			break;
		      }
		    else if( op == 'q' )
		      {
			DVDCloseFile( dvd_file );
			DVDClose( dvd );
			exit( 1 );
		      }
		    else
		      {
			fprintf( stderr, "\n[Hint] Please choose [o]verwrite, [a]ppend, [q]uit the next time ;-)\n" );
		      }
		  }
	      }
	    else
	      {
		/*assign the stream */
#if defined( HAS_LARGEFILE )
		if( ( streamout = open( name, O_WRONLY | O_CREAT | O_LARGEFILE, 0644 ) ) < 0 )
#else
		  if( ( streamout = open( name, O_WRONLY | O_CREAT, 0644 ) ) < 0 )
#endif
		    {
		      fprintf( stderr, "\n[Error] Error opening file %s\n", name );
		      exit ( 1 );
		    }
	      }
	}
      
      if( stdout_flag ) /*this writes to stdout*/
	{
	  streamout = STDOUT_FILENO; /*in other words: 1, see "man stdout" */
	}
      
      /* this here is the main copy part */	
      
      fprintf( stderr, "\n" );
      memset( bufferin, 0, BLOCK_COUNT * DVD_VIDEO_LB_LEN * sizeof( unsigned char ) );

/*       for ( ; ( offset + ( off_t ) seek_start ) < ( ( off_t ) file_size_in_blocks - ( off_t ) stop_before_end )   */
/* 	     && offset - ( off_t )max_filesize_in_blocks_summed < max_filesize_in_blocks;  */
/* 	    offset++ ) */
/* 	{ */

      file_block_count = block_count;
      for ( ; ( offset + ( off_t ) seek_start ) < ( ( off_t ) file_size_in_blocks - ( off_t ) stop_before_end )  
	     && offset - ( off_t )max_filesize_in_blocks_summed - (off_t)angle_blocks_skipped < max_filesize_in_blocks; 
	    offset += file_block_count )
	{
         /* Only read and write as many blocks as there are left in the file */
         if ( ( offset + file_block_count + ( off_t ) seek_start ) > ( ( off_t ) file_size_in_blocks - ( off_t ) stop_before_end ) )
           {
             file_block_count = ( off_t ) file_size_in_blocks - ( off_t ) stop_before_end - offset - ( off_t ) seek_start;
           }
         if ( offset + file_block_count - ( off_t )max_filesize_in_blocks_summed - (off_t)angle_blocks_skipped > max_filesize_in_blocks )
           {
             file_block_count = max_filesize_in_blocks - ( offset + file_block_count - ( off_t )max_filesize_in_blocks_summed - (off_t)angle_blocks_skipped );
           }

	  blocks = DVDReadBlocks( dvd_file,( offset + seek_start ), file_block_count, bufferin );
	  if( write( streamout, bufferin, DVD_VIDEO_LB_LEN * blocks ) < 0 )
	    {
	      fprintf( stderr, "\n[Error] Write() error\n"
		      "[Error] It's possible that you try to write files\n"
		      "[Error] greater than 2GB to filesystem which\n"
		      "[Error] doesn't support it? (try without -l)\n" );
	      fprintf( stderr, "[Error] Error: %s\n", strerror( errno ) );
	      exit( 1 );
	    }
	  l += blocks;

/*this is for people who report that it takes vobcopy ages to copy something */
/* TODO */


	  if( l > 100 )
	    {
	      /*this is the progress indicator*/
	      fprintf( stderr, "%4.0fMB of %4.0fMB written (%.0f %%)\r", 
		     ( float ) offset/512, 
		     ( float ) ( file_size_in_blocks - seek_start - stop_before_end )/512 ,
		     ( float ) ( offset*100 )/( file_size_in_blocks - seek_start - stop_before_end ) );
	      l=0;
	    }
	}
                  if( !stdout_flag )
                      {
                         if( fdatasync( streamout ) < 0 )
                           {
                             fprintf( stderr, "\n[Error] error writing to %s \n", name );
                             fprintf( stderr, "[Error] error: %s\n", strerror( errno ) );
                             exit( 1 );
                           }

                          close( streamout );  
                          
                          if( verbosity_level >= 1 )
                              {
                                  fprintf( stderr,"[Info] max_filesize_in_blocks %8.0f \n", ( float ) max_filesize_in_blocks );
                                  fprintf( stderr,"[Info] offset at the end %8.0f \n", ( float ) offset );
                                  fprintf( stderr,"[Info] file_size_in_blocks %8.0f \n",( float ) file_size_in_blocks );
                              }
                          /* now lets see whats the size of this file in bytes */
                          stat( name, &buf );
                          disk_vob_size += ( off_t ) buf.st_size;
                          
                          
                          if( large_file_flag && !cut_flag )
                              {
                                  if( ( vob_size - disk_vob_size ) < MAX_DIFFER )	 
                                      {
                                          re_name( name );
                                      }
                                  else
                                      {
                                          fprintf( stderr, "\n[Error]File size (%.0f) of %s differs largely from that on dvd, therefore keeps it's .partial\n", ( float ) buf.st_size, name );
                                      }
                              }
                          else if( !cut_flag )
                              {
                                          re_name( name );
                                           /* TODO: this check should go in again
                                  if( ( ( 1048571 * 2048 ) - disk_vob_size ) < MAX_DIFFER )
                                      {
                                          re_name( name );
                                      }
                                  else
                                      {
                                          fprintf( stderr, "\n[Error]File size (%.0f) of %s differs largely from 2GB, therefore keeps it's .partial\n", ( float ) buf.st_size, name );
                                      } */
                              }

                          else if( cut_flag )
                              {
                                  re_name( name );
                              }
                      
                          if( verbosity_level >= 1 )
                              {
                                  fprintf( stderr, "[Info] Single file size (of copied file %s ) %.0f\n", name, ( float ) buf.st_size );
                                  fprintf( stderr, "[Info] Cumulated size %.0f\n", ( float ) disk_vob_size );
                              }
                      }
       max_filesize_in_blocks_summed += max_filesize_in_blocks;
       fprintf( stderr, "[Info] Successfully copied file %s\n", name );
      j++; 	/* # of seperate files we have written */
    }
  /*end of main copy loop*/


  if( verbosity_level >= 1 )
    fprintf( stderr, "[Info] # of separate files: %i\n", j );
  
  /*
   * clean up and close everything 
   */

  ifoClose( vts_file );
  ifoClose( vmg_file );
  DVDCloseFile( dvd_file );
  DVDClose( dvd );
  fprintf( stderr,"\n[Info] Copying finished! Let's see if the sizes match (roughly)\n" );   
  fprintf( stderr,"[Info] Combined size of title-vobs: %.0f (%.0f MB)\n", ( float ) vob_size, ( float ) vob_size / ( 1024*1024 ) );
  fprintf( stderr,"[Info] Copied size (size on disk):  %.0f (%.0f MB)\n", ( float ) disk_vob_size, ( float ) disk_vob_size / ( 1024*1024 ) );
  
  if ( ( vob_size - disk_vob_size ) > MAX_DIFFER )
    {
      fprintf( stderr, "[Error] Hmm, the sizes differ by more than %d\n", MAX_DIFFER );
      fprintf( stderr, "[Hint] Take a look with MPlayer if the output is ok\n" );
    }
  else
    {
      fprintf( stderr, "[Info] Everything seems to be fine, the sizes match pretty good ;-)\n" );
      fprintf( stderr, "[Hint] Have a lot of fun!\n" );
    }
  
  return 0;
}
/*
 *this is the end of the main program, following are some useful functions
 * functions directly concerning the dvd have been moved to dvd.c
 */



/*
 * if you symlinked a dir to some other place the path name might not get
 * ended by a slash after the first tab press, therefore here is a / added
 * if necessary
 */

int add_end_slash( char *path )  
{  /* add a trailing '/' to path */
  char *pointer;
  if ( path[strlen( path )-1] != '/' )
    {
      pointer = path + strlen( path );
      *pointer = '/';
      pointer++;
      *pointer = '\0';
    }
  return 0;
}

/*
 * get available space on target filesystem
 */

off_t get_free_space( char *path, int verbosity_level )
{

#ifdef USE_STATFS
  struct statfs     buf1;
#else 
  struct statvfs    buf1; 
#endif 
/*   ssize_t temp1, temp2; */
  long temp1, temp2;
  off_t sum;
#ifdef USE_STATFS
  statfs( path, &buf1 );
  if( verbosity_level >= 1 )
    fprintf( stderr, "[Info] Used the linux statfs\n" );
#else
  statvfs( path, &buf1 );
  if( verbosity_level >= 1 )
    fprintf( stderr, "[Info] Used statvfs\n" );
#endif
  temp1 = buf1.f_bavail;
  /* On Solaris at least, f_bsize is not the actual block size -- lb */
  /* There wasn't this ifdef below, strange! How could the linux statfs 
   * handle this since there seems to be no frsize??! */
#ifdef USE_STATFS
   temp2 = buf1.f_bsize;
#else
  temp2 = buf1.f_frsize;
#endif
  sum = ( ( off_t )temp1 * ( off_t )temp2 );
  if( verbosity_level >= 1 )
    {
      fprintf( stderr, "[Info] In freespace_getter:for %s : %.0f free\n", path, ( float ) sum );
      fprintf( stderr, "[Info] In freespace_getter:bavail %ld * bsize %ld = above\n", temp1, temp2 );
    }
  /*   return ( buf1.f_bavail * buf1.f_bsize ); */
  return sum;
}


/*
 * get used space on dvd (for mirroring)
 */

off_t get_used_space( char *path, int verbosity_level )
{

#ifdef USE_STATFS
  struct statfs     buf2;
#else 
  struct statvfs    buf2; 
#endif 
/*   ssize_t temp1, temp2; */
  long temp1, temp2;
  off_t sum;
#ifdef USE_STATFS
  statfs( path, &buf2 );
  if( verbosity_level >= 1 )
    fprintf( stderr, "[Info] Used the linux statfs\n" );
#else
  statvfs( path, &buf2 );
  if( verbosity_level >= 1 )
    fprintf( stderr, "[Info] Used statvfs\n" );
#endif
  temp1 = buf2.f_blocks;
  /* On Solaris at least, f_bsize is not the actual block size -- lb */
  /* There wasn't this ifdef below, strange! How could the linux statfs 
   * handle this since there seems to be no frsize??! */
#ifdef USE_STATFS
   temp2 = buf2.f_bsize;
#else
  temp2 = buf2.f_frsize;
#endif
  sum = ( ( off_t )temp1 * ( off_t )temp2 );
  if( verbosity_level >= 1 )
    {
      fprintf( stderr, "[Info] In usedspace_getter:for %s : %.0f used\n", path, ( float ) sum );
      fprintf( stderr, "[Info] In usedspace_getter:part1 %ld, part2 %ld\n", temp1, temp2 );
    }
  /*   return ( buf1.f_blocks * buf1.f_bsize ); */
  return sum;
}

/*
 * this function concatenates the given information into a path name
 */

int make_output_path( char *pwd,char *name,int get_dvd_name_return, char *dvd_name,int titleid, int partcount )
{
  char temp[3];
  strcpy( name, pwd );
  if( !get_dvd_name_return )
    {
      strcat( name, dvd_name );
    }
  else
    strcat( name, "the_video_you_wanted" );
  
  sprintf( temp, "%d", titleid );
  strcat( name, temp );
  if( partcount >= 0 )
      {
          strcat( name, "-" );
          sprintf( temp, "%d", partcount );
          strcat( name, temp );
      }
  strcat( name, ".vob" );
  fprintf( stderr, "\n[Info] Outputting to %s", name );
  return 0;
}

/*
 *The usage function
 */

void usage( char *program_name ){
    fprintf( stderr, "Vobcopy "VERSION" - GPL Copyright (c) 2001 - 2006 robos@muon.de\n" 
            "\nUsage: %s \n"
            "[-m (mirror)] \n" 
	    "[-i /path/to/the/mounted/dvd/]\n" 
            "[-n title-number] \n"
            "[-t <your name for the dvd>] \n" 
            "[-o /path/to/output-dir/ (can be \"stdout\" or \"-\")] \n" 
            "[-f (force output)]\n"
	    "[-V (version)]\n"
            "[-v (verbose)]\n" 
            "[-v -v (create log-file)]\n" 
            "[-h (this here ;-)] \n"
            "[-I (infos about title, chapters and angles on the dvd)]\n"
            "[-1/path/to/second/output/dir/] [-2/.../third/..] [-3/../] [-4 /../]\n"
            "[-b <skip-size-at-beginning[bkmg]>] \n" 
            "[-e <skip-size-at-end[bkmg]>]\n"
            "[-O <single_file_name1,single_file_name2, ...>] \n" 
            "[-q (quiet)]\n"
            "[-F <fast-factor:1..64>]\n", program_name );

#if defined( HAS_LARGEFILE )
  fprintf( stderr, "[-l (large-file support for files > 2GB)] \n" );
#endif
  exit( 1 );
}


/* from play_title */
/**
 * Returns true if the pack is a NAV pack.  This check is clearly insufficient,
 * and sometimes we incorrectly think that valid other packs are NAV packs.  I
 * need to make this stronger.
 */
int is_nav_pack( unsigned char *buffer )
{
    return ( buffer[ 41 ] == 0xbf && buffer[ 1027 ] == 0xbf );
}

/*
*Rename: move file name from bla.partial to bla or mark as dublicate 
*/

void re_name( char *output_file ){
    char new_output_file[ 255 ];
    strcpy( new_output_file, output_file );
    new_output_file[ strlen( new_output_file ) - 8 ] = 0;

    if( ! link( output_file, new_output_file ) )
        {
            if( unlink( output_file ) )
                {
                    fprintf( stderr, "[Error] Could not remove old filename: %s \n", output_file );
                    fprintf( stderr, "[Hint] This: %s is a hardlink to %s. Dunno what to do... \n", new_output_file, output_file );
                }
            else
                fprintf( stderr, "[Info] Removed \".partial\" from %s since it got copied in full \n", output_file );
        }
    else
        {
            if( errno == EEXIST && ! overwrite_flag )
                {
                    fprintf( stderr, "[Error] File %s already exists! Gonna name the new one %s.dupe \n", new_output_file, new_output_file );
                    strcat( new_output_file, ".dupe" );
                    rename( output_file,  new_output_file );
                }
            if( errno == EEXIST && overwrite_flag )
                {
                    rename( output_file,  new_output_file );
                }

            if( errno == EPERM ) /*EPERM means that the filesystem doesn't allow hardlinks, e.g. smb */
                {
                    /*this here is a stdio function which simply overwrites an existing file. Bad but I don't want to include another test...*/
                    rename( output_file, new_output_file );
                fprintf( stderr, "[Info] Removed \".partial\" from %s since it got copied in full \n", output_file );
                }
        }
    /* test, test... */
    if( strstr( name, ".partial" ) )
       name[ strlen( name ) - 8 ] = 0;

}


/*
* Creates a directory with the given name, checking permissions and reacts accordingly (bails out or asks user)
*/

int makedir( char *name ){
    if( mkdir( name, 0777 ) )
        {
            if( errno == EEXIST )
                {
                    fprintf( stderr, "[Error] The directory %s\n already exists! \n", name );
                    fprintf( stderr, "[Hint] You can either [c]ontinue writing to it or you can [q]uit: " );
                    while ( 1 )
                        {
                            char op;
//                            op=fgetc( stdin );
                            while ((op = fgetc (stdin)) == EOF)
                                usleep (1);
                            fgetc ( stdin ); /* probably need to do this for second time it 
                            comes around this loop */
                            if( op == 'c' )
                                {
                                    return 0;
                                }
                            else if( op == 'q' )
                                {
                                    exit( 1 );
                                }
                            else
                                {
                                    fprintf( stderr, "\n[Hint] please choose [c]ontinue or [q]uit the next time ;-)\n" );
                                }
                        }
                    
                }
            else /*most probably the user has no right to create dir or space if full or something */
                {
                    fprintf( stderr, "[Error] Creating of directory %s\n failed! \n", name );
                    fprintf( stderr, "[Error] error: %s\n",strerror( errno ) );
                    exit( 1 );
                }
        }
    return 0;
}