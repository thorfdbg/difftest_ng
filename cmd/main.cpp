/*************************************************************************
** Written by Thomas Richter (THOR Software) for Accusoft	        **
** All Rights Reserved							**
**************************************************************************

This source file is part of difftest_ng, a universal image measuring
and conversion framework.

    difftest_ng is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    difftest_ng is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with difftest_ng.  If not, see <http://www.gnu.org/licenses/>.

*************************************************************************/
/*
 * Main program
 * 
 * $Id: main.cpp,v 1.117 2022/08/12 06:54:57 thor Exp $
 *
 * This class defines the main program and argument parsing.
 */

/// Includes
#include "std/stdio.hpp"
#include "cmd/main.hpp"
#include "std/string.hpp"
#include "std/assert.hpp"
#include "std/stdlib.hpp"
#include "std/math.hpp"
#include "interface/types.hpp"
#include "diff/psnr.hpp"
#include "diff/pre.hpp"
#include "diff/diffimg.hpp"
#include "diff/suppress.hpp"
#include "diff/fftimg.hpp"
#include "diff/restrict.hpp"
#include "diff/restore.hpp"
#include "diff/thres.hpp"
#include "diff/compare.hpp"
#include "diff/maxfreq.hpp"
#include "diff/fftfilt.hpp"
#include "diff/dimension.hpp"
#include "diff/histogram.hpp"
#include "diff/colorhist.hpp"
#include "diff/convertimg.hpp"
#include "diff/invert.hpp"
#include "diff/flip.hpp"
#include "diff/flipextend.hpp"
#include "diff/shift.hpp"
#include "diff/scale.hpp"
#include "diff/crop.hpp"
#include "diff/mrse.hpp"
#include "diff/ycbcr.hpp"
#include "diff/xyz.hpp"
#include "diff/sim2.hpp"
#include "diff/mask.hpp"
#include "diff/stripe.hpp"
#include "diff/add.hpp"
#include "diff/peakpos.hpp"
#include "diff/mapping.hpp"
#include "diff/downsampler.hpp"
#include "diff/upsampler.hpp"
#include "diff/clamp.hpp"
#include "diff/fill.hpp"
#include "diff/paste.hpp"
#include "diff/bayerconv.hpp"
#include "diff/debayer.hpp"
#include "diff/bayercolor.hpp"
#include "diff/tobayer.hpp"
#include "diff/whitebalance.hpp"
#include "diff/fromgrey.hpp"
#include "diff/butterfly.hpp"
#include "img/imglayout.hpp"
#include "img/imgspecs.hpp"
#include <new>
///

/// Usage
// Print the usage of this program.
void Usage(const char *progname)
{
  fprintf(stderr,"Usage: %s [options] original distorted\n"
	  "where original and distorted are ppm,pbm,pgm,pfm,pfs,bmp,pgx,tif,png,exr,rgbe or raw (craw,v12,yuv) images\n"
	  "and options are one or more of\n"
	  "--psnr             : measure the psnr with equal weights over all components\n"
	  "--maxsnr           : measure the snr with equal weights over all components, normalize to maximum signal\n"
	  "--snr              : measure the snr with equal weights over all components, normalize to source energy\n"
	  "--mse              : measure the mean square error with equal weights over all components\n"
	  "--rmse             : measure the root mean square error with equal weights over all components\n"
	  "--minpsnr          : measure the minimum psnr over all components\n"
	  "--ycbcrpsnr        : measure the psnr with weights derived from the YCbCr transformation\n"
	  "--yuvpsnr          : measure the psnr with weights derived from the YUV transformation\n"
	  "--swpsnr           : measure the psnr with weights coming from the subsampling factors\n"
	  "--mrse             : measure the log of the mean relative square error with equal weights\n"
	  "--minmrse          : measure the minimum mrse over all components\n"
	  "--ycbcrmrse        : measure the mrse with weights derived from the YCbCr transformation\n"
	  "--yuvmrse          : measure the mrse with weights derived from the YUV transformation\n"
	  "--swmrse           : measure the mrse with weights coming from the subsampling factors\n"
	  "--peak             : measure the peak relative error in dB over all components\n"
	  "--avgpeak          : measure the peak relative error averaged over all components, in dB\n"
	  "--peakx            : find the x position of the largest pixel error\n"
	  "--peaky            : find the y position of the largest pixel error\n"
	  "--min              : find the minimum value of original - distorted\n"
	  "--max              : find the maximum value of original - distorted\n"
	  "--toe              : find the minimum of the original\n"
	  "--head             : find the maximum of the original\n"
	  "--drift            : find the mean error (drift) between original and distorted\n"
	  "--mae              : find the mean absolute error\n"
	  "--pae              : find the peak absolute error\n"
	  "--stripe           : measure a striping indicator that detects horizontal or vertical artifacts\n"
	  "--width            : print the width of the images\n"
	  "--height           : print the height of the images\n"
	  "--depth            : print the number of components of the images\n"
	  "--precision comp   : print the bit precision of the given component\n"
	  "--signed    comp   : print the signedness of the given component (-1 = signed, +1 = unsigned)\n"
	  "--float     comp   : print whether the indicated component is IEEE floating point (1 = yes, 0 = no)\n"
	  "--diff target      : save the difference image (-i is an alternative form of this option)\n"
	  "--rawdiff target   : similar to --diff, except that it doesn't scale the difference to maximum range\n"
	  "--sdiff scale trgt : generate a differential signal with an explicitly given scale\n"
	  "--butterfly target : merge source and destination image in butterfly style\n"
	  "--suppress thres   : suppress all pixels in the target that are less than a threshold away from the source\n"
	  "--mask roi         : mask the source image by the mask image before applying the comparison\n"
	  "--notmask roi      : mask the source image by the inverse of the mask\n"
	  "--convert target   : save the original image unaltered, but possibly in a new format\n"
	  "--merge target     : merge the two images together, add second as components of first\n"
#ifdef USE_GSL
	  "--fft target       : save the fft of the difference image\n"
	  "--wfft target      : save the windowed fft of the difference image\n"
	  "--filt x y r dst   : run a radial filter around frequency x,y with radius r, saves the filtered image as dst\n"
	  "--nfilt x y r dst  : similar to --filt, but the output is normalized to the full range\n"
	  "--comb x y r dst   : apply a comb filter in direction x y and radius r\n"
	  "--ncomb x y r dst  : similar to --comb, but the output is normalized to the full range\n"
#endif
	  "--hist target      : generate a histogram plot. If \"target\" is -, write to stdout\n"
	  "--thres threshold  : compute the ratio of pixels whose difference is > than threshold\n"
	  "--colorhist size   : generate reduced histogram separately for each component using the given bucket size\n"
#ifdef USE_GSL
	  "--maxfreqr         : locate the absolute value of the most exposed frequency in the error image\n"
	  "--maxfreqx         : locate the horizontal component of the most exposed frequency in the error image\n"
	  "--maxfreqy         : locate the vertical component of the most exposed frequency in the error image\n"
	  "--maxfreqv         : compute the domination ratio of the most exposed frequency in the error image\n"
	  "--patternidx       : scan the FFT for suspicious patterns and output the likeliness of errors\n"
#endif
	  "--toflt dst        : save a floating point version of the source image\n"
	  "--asflt            : convert to floating point before proceeding (run as filter)\n"
	  "--tohfl dst        : save a half-float version of the source image\n"
	  "--tohfl            : convert to half-float before proceeding (run as filter)\n"
	  "--touns bpp dst    : save an unsigned integer version with bpp bits per pixel of the source image\n"
	  "--tosgn bpp dst    : save a signed integer version with bpp bits per pixel of the source image\n"
	  "--asuns bpp        : convert the two input images to unsigned bpp before further procesing\n"
	  "--assgn bpp        : convert the two input images to signed bpp before further processing\n"
	  "--gamma bpp gamma dst         : perform a gamma correction on a floating point image to create\n"
	  "                                integer output\n"
	  "--invgamma gamma dst          : perform an inverse gamma correction creating a floating point image\n"
	  "                                from integer\n"
	  "--toegamma slope gamma dst    : perform a gamma transformation on the same data type with a given slope in\n"
	  "                                its toe region\n"
	  "--invtoegamma slope gamma dst : perform an inverse gamma transformation on the same data with parameters\n"
	  "                                as above\n"
	  "--halflog dst      : represent a floating point image in IEEE half float format saved as 16 bit integers\n"
	  "--halfexp dst      : read a 16-bit integer image using IEEE half float and save as floating point\n"
	  "--togamma bpp gamma: convert both images to gamma before applying the measurement (apply as a filter)\n"
	  "--fromgamma gamma  : convert both images from a gamma before applying the measurement\n"
	  "--totoegamma slope gamma      : perform a forwards gamma transformation (linear to gamma) with given\n"
	  "                                slope in the toe region\n"
	  "--fromtoegamma slope gamma    : perform an inverse gamma transformation (gamma to linear) with given\n"
	  "                                slope in the toe region\n"
	  "--tohalflog        : convert to 16-bit integer before comparing (apply as filter)\n"
	  "--fromhalflog      : convert from 16-bit integer to float before comparing (apply as filter)\n"
	  "--tolog clamp      : convert to logarithmic domain with clamp value before comparing\n"
	  "--topercept        : convert from absolute luminance to a perceptually uniform space\n"
	  "--topq bits        : convert floating point to SMPTE 2084 quantized to the given bits\n"
	  "--frompq           : convert SMPTE 2084 quantized data to luminances\n"
	  "--tohlg bits       : convert floating point to Hybrid Log Gamma with HEVC conventions\n"
	  "--fromhlg          : convert Hybrid Log Gamma to linear luminance, 1000 nits peak\n"
	  "--invert           : invert the source image before comparing\n"
	  "--flipx            : flip the source horizontally before comparing\n"
	  "--flipy            : flip the source vertically before comparing\n"
	  "--flipxextend      : create a twice as wide image by flipping it over the right edge\n"
	  "--flipyextend      : create a twice as high image by flippingit over the bottom edge\n"
	  "--shift dx dy      : shift the image by the given amount right/bottom (or right/up if < 0)\n"
	  "--pad bpp dst      : pad (right-aligned) a component into a larger bit-depths\n"
	  "--asprec bpp       : set the bit-depth to bpp, padding input and output into the target bitdepth\n"
	  "--sub x y          : subsample all components by the subsampling factors in x and y direction\n"
	  "--csub x y         : subsample all but component 0 by the subsampling factors in x and y direction\n"
	  "--up x y           : upsample all components by the subsampling factors in x and y direction\n"
	  "--up auto          : upsample all components such that we get consistent 1x1 (444) sampling\n"
	  "--cup x y          : upsample all but component 0 by the subsampling factors in x and y direction\n"
	  "--coup x y         : co-sited upsampling of all components in x and y direction\n"
	  "--coup auto        : co-sited upsampling, with automatic choice of upsampling factors\n"
	  "--cocup x y        : co-sited upsampling of the chroma components in x and y direction\n"
	  "--boxup x y        : upsample with a simple box filter\n"
	  "--boxcup x y       : upsample the chrome components with a simple box filter\n"
	  "--clamp min max    : clamp the image(s) to the specified range of sample values\n"
	  "--only component   : acts as a filter and restricts all following operations to the given component\n"
	  "--upto component   : restricts all following operations to components 0..component-1\n"
	  "--rgb              : restricts the activity to at most the first three components\n"
	  "--crop x1 y1 x2 y2 : crop a rectangular image region (x1,y1)-(x2,y2). Edges are inclusive.\n"
	  "--cropd x1 y1 x2 y2: crop a rectangular image region (x1,y1)-(x2,y2) from the distorted image only.\n"
	  "--restore          : un-do the restrictions of --crop and --only or --rgb\n"
	  "--toycbcr          : convert images to 601 YCbCr before comparing\n"
	  "--toycbcrbl        : convert images to 601 YCbCr before comparing, and include a black level\n"
	  "--tosignedycbcr    : convert images to 601 YCbCr with signed chroma components\n"
	  "--fromycbcr        : convert images from 601 YCbCr to RGB before comparing\n"
	  "--fromycbcrbl      : convert images from 601 YCbCr to RGB before comparing, and remove the black level\n"
	  "--toycbcr709       : convert images to 709 YCbCr before comparing\n"
	  "--toycbcr709bl     : convert images to 709 YCbCr before comaring, and include a black level\n"
	  "--fromycbcr709     : convert images from 709 YCbCr to RGB before comparing\n"
	  "--fromycbcr709bl   : convert images from 709 YCbCr to RGB before comparing, and remove the black level\n"
	  "--toycbcr2020      : convert images to 2020 YCbCr before comparing\n"
	  "--toycbcr2020bl    : convert images to 2020 YCbCr before comaring, and include a black level\n"
	  "--fromycbcr2020    : convert images from 2020 YCbCr to RGB before comparing\n"
	  "--fromycbcr2020bl  : convert images from 2020 YCbCr to RGB before comparing, and remove the black level\n"
	  "--fromgrey         : convert a grey-scale image to color by duplicating components\n"
	  "--torct            : convert an image with the RCT from JPEG 2000\n"
	  "--tosignedrct      : convert an image with the RCT from JPEG 2000, leaving chroma signed\n"
	  "--torctd           : convert a 4 component RGGB image to YCbCr+DeltaG with the RCT\n"
	  "--torctd1 agmnt    : convert a Bayer pattern with given arrangement with the above RCTD\n"
	  "--torctx agmnt     : convert a Bayer pattern with given arrangement to RCT with\n"
	  "                     an improved longer averaging filter for green\n"
	  "--toydgcgcox agmnt : convert a Bayer pattern with given arrangement with an extended version\n"
	  "                     of the YDgCgCo transformation\n"
	  "--to422rct         : convert a 422 image with green in component 0 to YCbCr\n"
	  "--to422signedrct   : convert a 422 image with green in component 0, leaving chroma signed\n"
	  "--fromrct          : convert an image back to RGB with the inverse RCT\n"
	  "--fromrctd         : convert a YCbCr+DeltaG to a four-component RGGB with the inverse RCTD\n"
	  "--fromrctd1 agmnt  : convert a YCbCr+DeltaG to a 1-component RGGB Bayer pattern\n"
	  "--fromrctx agmnt   : convert an RCTX-converted image to a Bayer pattern image with the\n"
	  "                     given sample arrangment\n"
	  "--fromydgcgcox agmt: convert a YDgCgCoX-converted image to a Bayer pattern image with the\n"
	  "                     given sample arrangement\n"
	  "--from422rct       : convert from YCbCr to RGB with green in channel 0\n"
	  "--todeltag         : convert RGGB to RGB+DeltaG\n"
	  "--toycgco          : convert an image with the YCgCo transformation\n"
	  "--tosignedycgco    : convert an image to YCgCo leaving chroma signed\n"
	  "--tocycbcod        : convert an RGGB image to YCgCo+DeltaG\n"
	  "--fromycgco        : convert an image back to RGB with the inverse YCgCo transformation\n"
	  "--fromycgcod       : convert a YCgCo+DeltaG to RGGB with the inverse RCT\n"
	  "--fromdeltag       : convert RGB+DeltaG to RGGB\n"
	  "--toxyz            : convert images from 709 RGB to XYZ before comparing\n"
	  "--fromxyz          : convert images from XYZ to 709 RGB before comparing\n"
	  "--2020toxyz        : convert images from 2020 RGB to XYZ before comparing\n"
	  "--fromxyzto2020    : convert images from XYZ to 2020 RGB before comparing\n"
	  "--tosim2           : convert images from XYZ to the SIM2 monitor encoding\n"
	  "--tolms            : convert images from RGB to LMS before comparing\n"
	  "--fromlms          : convert images from LMS to RGB before comparing\n"
	  "--xyztolms         : convert images from XYZ to LMS before comparing\n"
	  "--lmstoxyz         : convert images from LMS to XYZ before comparing\n"
	  "--scale a,b,c...   : scale components by the indicated factors before comparing\n"
	  "--offset a,b,c...  : offset component values by the indicated values before comparing\n"
	  "--tobayer          : convert a four-component image to a Bayer-pattern image\n"
	  "--frombayer        : convert a Bayer patterned grey-scale image to four components\n"
	  "--tobayersh   agmnt: convert a four-component image in component order RGGB\n"
	  "                     to a Bayer patterned image of the given arragement\n"
	  "--frombayersh agmnt: convert a Bayer patterned image in the given sample order\n"
	  "                     into a four-component image in RGGB order\n"
	  "--422tobayer  agmnt: convert a 422 three-component image to a Bayer pattern image\n"
	  "                     where the argument describes the sample organization. It can be either\n"
	  "                     grbg,rggb,gbrg or bggr, and green becomes the luma component\n"
	  "--bayerto422  agmnt: convert a Bayer pattern image to a 422 three component image with luma\n"
	  "                     as green and Cb as red and Cr as blue component. The argument describes\n"
	  "                     the bayer pattern arrangement as above.\n"
	  "--debayer     agmnt: de-Bayer a bayer pattern image with bi-linear interpolation, org describes\n"
	  "                     the sample organization and can be grbg,rggb,gbrg or bggr\n"
	  "--debayerahd  agmnt: de-Bayer with the Adaptive Homogeneity-Directed Demosaic Algorithm\n"
	  "--rgbtobayer  agmnt: create a 1 component Bayer pattern from an RGB image\n"
	  "--fill r,g,b,...   : fill the source image with the given color\n"
	  "--paste x y        : paste the distorted image at the given position into the source\n"
	  "--raw              : encode output in raw if applicable\n"
	  "--ascii            : encode output in ascii if applicable\n"
	  "--interleaved      : encode output in interleaved samples if applicable\n"
	  "--separate         : encode output in separate planes if applicable\n"
	  "--isyuv            : override automatic YUV detection, sources are really in YUV\n"
	  "--isrgb            : override automatic YUV detection, sources are really in RGB\n"
	  "--isfullrange      : override automatic range detection, source has no head/toe region\n"
	  "--isreducedrange   : override automatic range detection, source has head/toe region\n"
	  "--littleendian     : use little endian output if applicable\n"
	  "--bigendian        : use big endian output if applicable\n"
	  "--toabsradiance    : multiply floating point samples by recorded radiance scale to convert to absolute radiance\n"
	  "--brief            : use a brief (only numeric) output format\n"
	  ">,>=,==,!=,<=,< t  : last result must be larger, larger or equal, equal, not equal,\n"
	  "                     smaller or equal or smaller than given threshold t.\n"
	  "                     Attention: Quoting required when used from the shell.\n"
	  "\n"
	  "If the source image is '-/<width>x<height>x<depth>', it is replaced by a blank image of the\n"
	  "given dimensions. This image can be filled with any other color by --fill, see above.\n"
	  "If the distorted image file name equals '-', then the image is replaced by a blank image\n"
	  "\n"
	  "--help             : print this page\n"
	  "--rawhelp          : print help on raw image formatting. First time users: PLEASE READ THIS.\n"
	  "\n",
	  progname
	  );
}
///

/// RawHelp
// Print help on the formatting of raw images.
void RawHelp(void) 
{
  fprintf(stderr,
	  "Raw formats are unframed and hence the formatting of the file must be specified\n"
	  "on the command line, here as part of the FILE NAME, not by separate options.\n"
	  "\n"
	  "Raw files are specified by 'filename.raw@format', with an '@' (at) sign separating\n"
	  "the format specification from the file name. Note that the annex .raw is necessary.\n"
	  "\n"
	  "The format specification itself consists of two parts, the image dimensions and\n"
	  "the layout of the data:\n"
	  "\n"
	  "<width>x<height>x<depth>:<datalayout>\n"
	  "\n"
	  "where <width> is the width, <height> the height and <depth> the number of\n"
	  "components in the image, without the angle brackets (pure numerical values).\n"
	  "Numbers are separated by 'x' (lower-case x).\n"
	  "This part MAY be omitted for saving since image dimensions are known.\n"
	  "The colon ':', however, must be present.\n"
	  "\n"
	  "Image data can be either represented INTERLEAVED, that is, all components of a\n"
	  "single pixel are adjacent to each other, or SEPARATE, that is, each component is\n"
	  "described in a separate bitplane, and the bitplanes are stored adjacent to each\n"
	  "other. Additional padding bits might be present in the interleaved representation.\n"
	  "A single pixel is described by one or several fields, where each field encodes\n"
	  "the data of a single component or may simply be present for padding.\n"
	  "Fields can be either signed or unsigned, have a bit-width and an endianness.\n"
	  "\n"
	  "In the interleaved presentation, fields can be bit-packed together, i.e. may share\n"
	  "bits in a byte, word or longword. If fields are bit-packed, the entire number of\n"
	  "bits must be either 8,16 or 32, and shares the endianness of all its components,\n"
	  "i.e. all components must indicate the same endianness.\n"
	  "\n"
	  "In the separate presentation, bits of a component are packed near each other\n"
	  "without any padding. However, some components might be subsampled, i.e. may\n"
	  "contain less samples than others. This option does not exist for interleaved\n"
	  "data.\n"
	  "\n"
	  "The format specification looks like this for interleaved data:\n"
	  "\n"
	  "<packing>{<sign-flag><bits><endian>=<target>}:{<sign-flag><bits><endian>=<target>}:...\n"
	  "\n"
	  "where <packing> is either + or -, indicating the packing order within the field\n"
	  "      with +, which is the default, packing is from MSB to LSB, with '-' bits\n"
	  "      are packed from LSB to MSB\n"
	  "where the curly brackets indicate the interleaved format\n"
	  "where <sign-flag> is an optional '+' or '-' sign indicating whether the component\n"
	  "      is signed (then '-') or unsigned (then '+'). If omitted, the component is\n"
	  "      unsigned.\n"
	  "where <bits> is a mandatory number of bits the component takes, e.g. 8\n"
	  "where <endian> is an optional endian indicator. It is '+' for big-endian and '-'\n"
	  "      for little-endian data. If omitted, big-endian is assumed.\n"
	  "where <target> indicates to which component the data belongs, e.g. 0 for red\n"
	  "      the target is separated by an equals sign '=' from the endianness.\n"
	  "      For padding data, equal-sign and target are omitted.\n"
	  "where the colon ':' indicates that fields have separate endianness. This only\n"
	  "      works for fields of withs 8,16,32 or 64 bits.\n"
	  "\n"
	  "If the colon is replaced by a comma, the fields adjacent to the comma are bit-\n"
	  "packed into a single data unit. In total, up to 32 bits can be packed together,\n"
	  "and the total number of bits packed must be either 8, 16 or 32. Then, the endian-\n"
	  "ness of all fields packed together applies to the packed result, and must be\n"
	  "identical. By default, bit-packing reads from the MSB to the LSB. With the\n"
	  "optional minus sign in front, packing is from LSB to MSB.\n"
	  "Components may appear multiple times in the same format specification, which implies\n"
	  "subsampling. The subsampling factors are inclined from the sample count per component.\n"
	  "The sample pattern is filled at each line end, potentially creating dummy samples that\n"
	  "will be skipped over when reading and which are written as zero on writing.\n"
	  "\n"
	  "The format for the separate representation uses square brackets instead:\n"
	  "\n"
	  "<packing>[<sign-flag><bits><endian>=<target>]/<subx>x<suby>:...\n"
	  "\n"
	  "and the syntax is as above, except that subsampling factors can be added to a field\n"
	  "description. They are separated by a slash '/' from the field description, followed\n"
	  "by the horizontal and vertical subsampling factors, separated by an 'x'.\n"
	  "The separate format also allows to pack several components together into one field,\n"
	  "similar to the above, packed fields are separated by a comma instead of a semicolon.\n"
	  "In such a case, the subsampling of the packed channels must be consistent and\n"
	  "identical within the same plane.\n"
	  "A padding channel is indicated by a missing target specification.\n"
	  "\n"
	  "EXAMPLES:\n"
	  "\n"
	  "A 640x480 RGB image with 8 bits per component encoded as RRRRRRRRGGGGGGGGBBBBBBBB\n"
	  "is denoted as this:\n"
	  "\n"
	  "       image.raw@640x480x3:{8=0}:{8=1}:{8=2}\n"
	  "\n"
	  "Image dimensions can be omitted when saving, it then may be simplified to:\n"
	  "\n"
	  "      image.raw@:{8=0}:{8=1}:{8=2}\n"
	  "\n"
	  "Note both '@' and ':' must be present.\n"
	  "\n"
	  "\n"
	  "Typical image formats:\n"
	  "\n"
	  "{8=2}:{8=1}:{8=0}:           24 bits, pixel layout:\n"
	  "                             BBBBBBBBGGGGGGGGRRRRRRRR\n"
	  "{8=2}:{8=1}:{8=0}:{8}        32 bits, plus a pad byte:\n"
	  "                             BBBBBBBBGGGGGGGGRRRRRRRR00000000\n"
	  "{8=2}:{8=1}:{8=0}:{8=3}      32 bits plus alpha channel:\n"
	  "                             BBBBBBBBGGGGGGGGRRRRRRRRAAAAAAAA\n"
	  "[1=0]                        bit-packed 1bpp black & white image\n"
	  "[8=0]:[8=1]:[8=2]            YUV or RGB in separate encoding, three planes, each 8bpp\n"
	  "[8=0]:[8=1]/2x2:[8=2]/2x2    YUV 420 in separate planes\n"
	  "[8=0]:[8=1]/2x1:[8=2]/2x1    YUV 422 in separate planes\n"
	  "[4],[12=0]:[4]/2x1,[12=1]/2x1:[4]/2x1,[12=2]/2x1\n"
	  "                             YUV 422 in separate planes, 12 bits per component\n"
	  "                             where each component is packed into 16 bits with\n"
	  "                             padding bits upfront, represented in big-endian\n"
	  "[4-],[12-=0]:[4-]/2x1,[12-=1]/2x1:[4-]/2x1,[12-=2]/2x1\n"
	  "                             YUV 422 12 bits/component as above, but little-endian.\n"
	  "{2-},{10-=2},{10-=1},{10-=0} 32 bits, pixel layout is ten bits per component with\n"
	  "                             two padding bits in front, packed into 32 bits which\n"
	  "                             is written in little-endian format. Represented as\n"
	  "                             little endian, this format reads\n"
	  "                             00BBBBBBBBBBGGGGGGGGGGRRRRRRRRRR\n"
	  "                             but in the file, bytes are shuffled around:\n"
	  "                             RRRRRRRR GGGGGGRR BBBBGGGG 00BBBBBB\n"
	  "{1-},{5-=2},{5-=1},{5-=0}    16 bits, pixel layout is five bits per component with\n"
	  "                             a single pad-bit upfront, packed into a 16-bit word\n"
	  "                             which is written in little-endian format. On a little\n"
	  "                             endian machine, this reads as\n"
	  "                             0BBBBBGGGGGRRRRR\n"
	  "                             but in the file, bytes are ordered reversely:\n"
	  "                             GGGRRRRR 0BBBBBGG\n"
	  "-{10-=1},{10-=0},{10-=2},{2-}:-{10-=0},{10-=1},{10-=0},{2-}:\n"
	  "-{10-=2},{10-=0},{10-=1},{2-}:-{10-=0},{10-=2},{10-=0},{2-}\n"
	  "                             This (single) line creates a raw format that complies to the\n"
	  "                             V210 pixel format. Each component is 10 bits wide,\n"
	  "                             three samples are left-aligned into one 32 bit word\n"
	  "                             causing two padding bits per 32 bit word. Each 32 bit\n"
	  "                             word is in little endian, and filled from LSB to MSB.\n"
	  "                             The samping order is UYV - YUY - VYU - YVY.\n"
	  "                             Subsampling factors are derived from the sample counts\n"
	  "                             and padding is applied at the end of each line to one\n"
	  "                             complete cycle.\n"
	  "{6-},{10-=0}:{6-},{10-=1}:{6-},{10-=0}:{6-},{10-=2}:\n"
	  "                             pixel-interleaved yuv in little-endian, where samples are\n"
	  "                             packed as YUYV, each in a 16 bit little-endian format.\n"
	  "\n");
}
///

/// ParseDouble
double ParseDouble(const char *str)
{
  char *endptr;
  double val;

  if (str == NULL)
    throw "insufficient arguments";
    
  if (!strcmp(str,"inf") || !strcmp(str,"Inf"))
    return HUGE_VAL;

  val = strtod(str,&endptr);

  if (*endptr)
    throw "argument is not a number";

  return val;
}
///

/// ParseLong
long ParseLong(const char *str)
{
  char *endptr;
  long val;

  if (str == NULL)
    throw "insufficient arguments";
    

  val = strtol(str,&endptr,0);

  if (*endptr)
    throw "argument is not a number";

  return val;
}
///

/// ParseMetrics
// Parse all metrics/true measurement tools.
class Meter *ParseMetrics(int &,char **&argv)
{
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--psnr")) {
    return new class PSNR(PSNR::Mean);
  } else if (!strcmp(arg,"--maxsnr")) {
    return new class PSNR(PSNR::Mean,false,true);
  } else if (!strcmp(arg,"--snr")) {
    return new class PSNR(PSNR::Mean,false,true,true);
  } else if (!strcmp(arg,"--mse")) {
    return new class PSNR(PSNR::Mean,true);
  } else if (!strcmp(arg,"--rmse")) {
    return new class PSNR(PSNR::RootMean,true);
  } else if (!strcmp(arg,"--minpsnr")) {
    return new class PSNR(PSNR::Min);
  } else if (!strcmp(arg,"--ycbcrpsnr")) {
    return new class PSNR(PSNR::YCbCr);
  } else if (!strcmp(arg,"--yuvpsnr")) {
    return new class PSNR(PSNR::YUV);
  } else if (!strcmp(arg,"--swpsnr")) {
    return new class PSNR(PSNR::SamplingWeighted);
  } else if (!strcmp(arg,"--mrse")) {
    return new class MRSE(MRSE::Mean);
  } else if (!strcmp(arg,"--minmrse")) {
    return new class MRSE(MRSE::Min);
  } else if (!strcmp(arg,"--ycbcrmrse")) {
    return new class MRSE(MRSE::YCbCr);
  } else if (!strcmp(arg,"--yuvmrse")) {
    return new class MRSE(MRSE::YUV);
  } else if (!strcmp(arg,"--swmrse")) {
    return new class MRSE(MRSE::SamplingWeighted);
  } else if (!strcmp(arg,"--peak")) {
    return new class PRE(PRE::Min);
  } else if (!strcmp(arg,"--avgpeak")) {
    return new class PRE(PRE::Mean);
  } else if (!strcmp(arg,"--peakx")) {
    return new class PeakPos(PeakPos::PeakX);
  } else if (!strcmp(arg,"--peaky")) {
    return new class PeakPos(PeakPos::PeakY);
  } else if (!strcmp(arg,"--min")) {
    return new class Thres(Thres::Min);
  } else if (!strcmp(arg,"--max")) {
    return new class Thres(Thres::Max);
  } else if (!strcmp(arg,"--drift")) {
    return new class Thres(Thres::Drift);
  } else if (!strcmp(arg,"--mae")) {
    return new class Thres(Thres::Avg);
  } else if (!strcmp(arg,"--pae")) {
    return new class Thres(Thres::Peak);
  } else if (!strcmp(arg,"--toe")) {
    return new class Thres(Thres::Toe);
  } else if (!strcmp(arg,"--head")) {
    return new class Thres(Thres::Head);
  }

  return NULL;
}
///

/// ParseProperties
// Parse off elementary settings that return properties of the images.
class Meter *ParseProperties(int &argc,char **&argv)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--width")) {
    m = new class Dimension(Dimension::Width);
  } else if (!strcmp(arg,"--height")) {
    m = new class Dimension(Dimension::Height);
  } else if (!strcmp(arg,"--depth")) {
    m = new class Dimension(Dimension::Depth);
  } else if (!strcmp(arg,"--precision")) {
    if (argc < 3)
      throw "--precision requires a component number";
    m = new class Dimension(Dimension::Precision,ParseLong(argv[2]));
    argc--;
    argv++;
  } else if (!strcmp(arg,"--signed")) {
    if (argc < 3)
      throw "--signed requires a component number";
    m = new class Dimension(Dimension::Signed,ParseLong(argv[2]));
    argc--;
    argv++;
  } else if (!strcmp(arg,"--float")) {
    if (argc < 3)
      throw "--float requires a component number";
    m = new class Dimension(Dimension::Float,ParseLong(argv[2]));
    argc--;
    argv++;
  }
  
  return m;
}
///

/// ParseFFT
// Parse FFT related features that require the GSL
#ifdef USE_GSL
class Meter *ParseFFT(int &argc,char **&argv)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--fft")) {
    if (argc < 3)
      throw "--fft requires the target file name as argument";
    m = new class FFTImg(argv[2],false);
    argc--;
    argv++;	  
  } else if (!strcmp(arg,"--wfft")) {
    if (argc < 3)
      throw "--wfft requires the target file name as argument";
    m = new class FFTImg(argv[2],true);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--filt")) {
    long x,y,r;
    if (argc < 6)
      throw "--filt requires four arguments, x,y, radius and file name";
    x = ParseLong(argv[2]);
    y = ParseLong(argv[3]);
    r = ParseLong(argv[4]);
    m = new class FFTFilt(argv[5],x,y,r,false,0,0);
    argc -= 4;
    argv += 4;
  } else if (!strcmp(arg,"--nfilt")) {
    long x,y,r;
    if (argc < 6)
      throw "--nfilt requires four arguments, x,y, radius and file name";
    x = ParseLong(argv[2]);
    y = ParseLong(argv[3]);
    r = ParseLong(argv[4]);
    m = new class FFTFilt(argv[5],x,y,r,true,0,0);
    argc -= 4;
    argv += 4;
  } else if (!strcmp(arg,"--comb")) {
    long x,y,r;
    if (argc < 6)
      throw "--comb requires four arguments, x,y, radius and file name";
    x = ParseLong(argv[2]);
    y = ParseLong(argv[3]);
    r = ParseLong(argv[4]);
    m = new class FFTFilt(argv[5],0,0,r,false,x,y);
    argc -= 4;
    argv += 4;
  } else if (!strcmp(arg,"--ncomb")) {
    long x,y,r;
    if (argc < 6)
      throw "--ncomb requires four arguments, x,y, radius and file name";
    x = ParseLong(argv[2]);
    y = ParseLong(argv[3]);
    r = ParseLong(argv[4]);
    m = new class FFTFilt(argv[5],0,0,r,true,x,y);
    argc -= 4;
    argv += 4;
  } else if (!strcmp(arg,"--maxfreqr")) {
    m = new class MaxFreq(MaxFreq::MaxR);
  } else if (!strcmp(arg,"--maxfreqx")) {
    m = new class MaxFreq(MaxFreq::MaxH);
  } else if (!strcmp(arg,"--maxfreqy")) {
    m = new class MaxFreq(MaxFreq::MaxV);
  } else if (!strcmp(arg,"--maxfreqv")) {
    m = new class MaxFreq(MaxFreq::Var);
  } else if (!strcmp(arg,"--maxfreqv")) {
    m = new class MaxFreq(MaxFreq::Var);
  } else if (!strcmp(arg,"--patternidx")) {
    m = new class MaxFreq(MaxFreq::Pattern);
  }

  return m;
}
#endif
///

/// ParseColor
// Parse conversions between various color spaces.
class Meter *ParseColor(int &,char **&argv,struct ImgSpecs &specout)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--toycbcr")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::YCbCr_Trafo);
  } else if (!strcmp(arg,"--toycbcrbl")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,true,YCbCr::YCbCr_Trafo);
  } else if (!strcmp(arg,"--tosignedycbcr")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,true,false,YCbCr::YCbCr_Trafo);
  } else if (!strcmp(arg,"--fromycbcr")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false, false,YCbCr::YCbCr_Trafo);
  } else if (!strcmp(arg,"--fromycbcrbl")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false, true,YCbCr::YCbCr_Trafo);
  } else if (!strcmp(arg,"--torct")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::RCT_Trafo);
  } else if (!strcmp(arg,"--tosignedrct")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,true,false,YCbCr::RCT_Trafo);
  } else if (!strcmp(arg,"--torctd")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::RCTD_Trafo);
  } else if (!strcmp(arg,"--to422rct")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::RCT422_Trafo);
  } else if (!strcmp(arg,"--tosigned422rct")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,true,false,YCbCr::RCT422_Trafo);
  } else if (!strcmp(arg,"--fromrct")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false,false,YCbCr::RCT_Trafo);
  } else if (!strcmp(arg,"--fromrctd")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false,false,YCbCr::RCTD_Trafo);
  } else if (!strcmp(arg,"--from422rct")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false,false,YCbCr::RCT422_Trafo);
  } else if (!strcmp(arg,"--toycgco")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::YCgCo_Trafo);
  } else if (!strcmp(arg,"--tosignedycgco")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,true,false,YCbCr::YCgCo_Trafo);
  } else if (!strcmp(arg,"--toycgcod")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::YCgCoD_Trafo);
  } else if (!strcmp(arg,"--todeltag")) {
    m = new YCbCr(false,false,false,YCbCr::Delta_Trafo);
  } else if (!strcmp(arg,"--fromycgco")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false,false,YCbCr::YCgCo_Trafo);
  } else if (!strcmp(arg,"--fromycgcod")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false,false,YCbCr::YCgCoD_Trafo);
  } else if (!strcmp(arg,"--fromdeltag")) {
    m = new YCbCr(true ,false,false,YCbCr::Delta_Trafo);
  } else if (!strcmp(arg,"--toycbcr709")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::YCbCr709_Trafo);
  } else if (!strcmp(arg,"--toycbcr709bl")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,true,YCbCr::YCbCr709_Trafo);
  } else if (!strcmp(arg,"--fromycbcr709")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false, false,YCbCr::YCbCr709_Trafo);
  } else if (!strcmp(arg,"--fromycbcr709bl")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false, true,YCbCr::YCbCr709_Trafo);
  } else if (!strcmp(arg,"--toycbcr2020")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,false,YCbCr::YCbCr2020_Trafo);
  } else if (!strcmp(arg,"--toycbcr2020bl")) {
    specout.YUVEncoded = ImgSpecs::Yes;
    m = new YCbCr(false,false,true,YCbCr::YCbCr2020_Trafo);
  } else if (!strcmp(arg,"--fromycbcr2020")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false, false,YCbCr::YCbCr2020_Trafo);
  } else if (!strcmp(arg,"--fromycbcr2020bl")) {
    specout.YUVEncoded = ImgSpecs::No;
    m = new YCbCr(true ,false, true,YCbCr::YCbCr2020_Trafo);
  } else if (!strcmp(arg,"--toxyz")) {
    m = new XYZ(XYZ::RGBtoXYZ,false);
  } else if (!strcmp(arg,"--fromxyz")) {
    m = new XYZ(XYZ::RGBtoXYZ,true);
  } else if (!strcmp(arg,"--2020toxyz")) {
    m = new XYZ(XYZ::RGB2020toXYZ,false);
  } else if (!strcmp(arg,"--fromxyzto2020")) {
    m = new XYZ(XYZ::RGB2020toXYZ,true);
  } else if (!strcmp(arg,"--tosim2")) {
    m = new Sim2();
  } else if (!strcmp(arg,"--tolms")) {
    m = new XYZ(XYZ::RGBtoLMS,false);
  } else if (!strcmp(arg,"--fromlms")) {
    m = new XYZ(XYZ::RGBtoLMS,true);
  } else if (!strcmp(arg,"--xyztolms")) {
    m = new XYZ(XYZ::XYZtoLMS,false);
  } else if (!strcmp(arg,"--lmstoxyz")) {
    m = new XYZ(XYZ::XYZtoLMS,true);
  } else if (!strcmp(arg,"--fromgrey")) {
    m = new class FromGrey();
  }

  return m;
}
///

/// ParseBayer
// Various functions for bayer pattern handling.
class Meter *ParseBayer(int &argc,char **&argv)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--tobayer")) {
    m = new BayerConv(true,false,false);
  } else if (!strcmp(arg,"--frombayer")) {
    m = new BayerConv(false,false,false);
  } else if (!strcmp(arg,"--tobayersh")) {
    if (argc < 3)
      throw "--tobayershuffle requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerConv(true,false,true,BayerConv::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerConv(true,false,true,BayerConv::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerConv(true,false,true,BayerConv::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerConv(true,false,true,BayerConv::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--frombayersh")) {
    if (argc < 3)
      throw "--fromayershuffle requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerConv(false,false,true,BayerConv::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerConv(false,false,true,BayerConv::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerConv(false,false,true,BayerConv::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerConv(false,false,true,BayerConv::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--422tobayer")) {
    if (argc < 3)
      throw "--422tobayer requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerConv(true,true,false,BayerConv::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerConv(true,true,false,BayerConv::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerConv(true,true,false,BayerConv::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerConv(true,true,false,BayerConv::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--bayerto422")) {
     if (argc < 3)
      throw "--bayerto422 requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerConv(false,true,false,BayerConv::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerConv(false,true,false,BayerConv::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerConv(false,true,false,BayerConv::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerConv(false,true,false,BayerConv::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--debayer")) {
    if (argc < 3)
      throw "--debayer requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new Debayer(Debayer::Bilinear,Debayer::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new Debayer(Debayer::Bilinear,Debayer::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new Debayer(Debayer::Bilinear,Debayer::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new Debayer(Debayer::Bilinear,Debayer::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--debayerahd")) {
    if (argc < 3)
      throw "--debayeradh requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new Debayer(Debayer::ADH,Debayer::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new Debayer(Debayer::ADH,Debayer::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new Debayer(Debayer::ADH,Debayer::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new Debayer(Debayer::ADH,Debayer::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--torctd1")) {
    if (argc < 3)
      throw "--torctd1 requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerColor(false,BayerColor::RCTD,BayerColor::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerColor(false,BayerColor::RCTD,BayerColor::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerColor(false,BayerColor::RCTD,BayerColor::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerColor(false,BayerColor::RCTD,BayerColor::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--fromrctd1")) {
    if (argc < 3)
      throw "--fromrctd1 requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerColor(true,BayerColor::RCTD,BayerColor::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerColor(true,BayerColor::RCTD,BayerColor::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerColor(true,BayerColor::RCTD,BayerColor::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerColor(true,BayerColor::RCTD,BayerColor::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--torctx")) {
    if (argc < 3)
      throw "--torctx requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerColor(false,BayerColor::RCTX,BayerColor::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerColor(false,BayerColor::RCTX,BayerColor::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerColor(false,BayerColor::RCTX,BayerColor::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerColor(false,BayerColor::RCTX,BayerColor::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--fromrctx")) {
    if (argc < 3)
      throw "--fromrctx requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerColor(true,BayerColor::RCTX,BayerColor::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerColor(true,BayerColor::RCTX,BayerColor::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerColor(true,BayerColor::RCTX,BayerColor::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerColor(true,BayerColor::RCTX,BayerColor::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--toydgcgcox")) {
    if (argc < 3)
      throw "--toydgcgcox requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerColor(false,BayerColor::YDgCoCgX,BayerColor::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerColor(false,BayerColor::YDgCoCgX,BayerColor::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerColor(false,BayerColor::YDgCoCgX,BayerColor::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerColor(false,BayerColor::YDgCoCgX,BayerColor::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--fromydgcgcox")) {
    if (argc < 3)
      throw "--fromydgcgcox requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new BayerColor(true,BayerColor::YDgCoCgX,BayerColor::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new BayerColor(true,BayerColor::YDgCoCgX,BayerColor::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new BayerColor(true,BayerColor::YDgCoCgX,BayerColor::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new BayerColor(true,BayerColor::YDgCoCgX,BayerColor::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  } else if (!strcmp(arg,"--rgbtobayer")) {
    if (argc < 3)
      throw "--rgbtobayer requires a string argument";
    if (!strcmp(argv[2],"grbg"))
      m = new ToBayer(ToBayer::GRBG);
    else if (!strcmp(argv[2],"rggb"))
      m = new ToBayer(ToBayer::RGGB);
    else if (!strcmp(argv[2],"gbrg"))
      m = new ToBayer(ToBayer::GBRG);
    else if (!strcmp(argv[2],"bggr"))
      m = new ToBayer(ToBayer::BGGR);
    else
      throw "unknown Bayer subpixel arrangement";
    argc--;
    argv++;
  }
  
  return m;
}
///

/// ParseConversions
// Various format conversions/alterations.
class Meter *ParseConversions(int &argc,char **&argv,struct ImgSpecs &specout)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--toflt")) {
    if (argc < 3)
      throw "--toflt requires a file name as argument";
    m = new class Scale(argv[2],false,true,false,false,32,false,specout);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--asflt")) {
    m = new class Scale(NULL,false,true,false,false,32,false,specout);
  } else if (!strcmp(arg,"--tohfl")) {
    if (argc < 3)
      throw "--tohlf requires a file name as argument";
    m = new class Scale(argv[2],false,true,false,false,16,false,specout);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--ashfl")) {
    m = new class Scale(NULL,false,true,false,false,16,false,specout);
  } else if (!strcmp(arg,"--touns")) {
    long bpp;
    if (argc < 4)
      throw "--touns requires a bit depth and a file name as argument";
    bpp = ParseLong(argv[2]);
    m   = new class Scale(argv[3],true,false,true,false,bpp,false,specout);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--asuns")) {
    long bpp;
    if (argc < 3)
      throw "--asuns requires a bit depth as argument";
    bpp = ParseLong(argv[2]);
    m   = new class Scale(NULL,true,false,true,false,bpp,false,specout);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--tosgn")) {
    long bpp;
    if (argc < 4)
      throw "--tosgn requires a bit depth and a file name as argument";
    bpp = ParseLong(argv[2]);
    m   = new class Scale(argv[3],true,false,false,true,bpp,false,specout);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--assgn")) {
    long bpp;
    if (argc < 3)
      throw "--assgn requires a bit depth as argument";
    bpp = ParseLong(argv[2]);
    m   = new class Scale(NULL,true,false,false,true,bpp,false,specout);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--pad")) {
    long bpp;
    if (argc < 4)
      throw "--pad requires a bit depth and a file name as argument";
    bpp = ParseLong(argv[2]);
    m   = new class Scale(argv[3],true,false,false,false,bpp,true,specout);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--asprec")) {
    long bpp;
    if (argc < 3)
      throw "--asprec requires a bit depth as argument";
    bpp = ParseLong(argv[2]);
    m   = new class Scale(NULL,true,false,false,false,bpp,true,specout);
    argc--;
    argv++;
  } 
  
  return m;
}
///

/// ParseTransferFunctions
// Parse various transfer function related methods.
class Meter *ParseTransferFunctions(int &argc,char **&argv,struct ImgSpecs &specout)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--gamma")) {
    long bpp;
    double gamma;
    if (argc < 5)
      throw "--gamma requires three arguments, a bit depth, a gamma value and a file name";
    bpp   = ParseLong(argv[2]);
    gamma = ParseDouble(argv[3]);
    m     = new class Mapping(argv[4],Mapping::Gamma,gamma,false,bpp,false,specout);
    argc -= 3;
    argv += 3;
  } else if (!strcmp(arg,"--toegamma")) {
    double gamma,toeslope;
    if (argc < 5)
      throw "--toegamma requires three arguments, a toe region slope, a gamma value and a file name";
    toeslope = ParseDouble(argv[2]);
    gamma    = ParseDouble(argv[3]);
    m        = new class Mapping(argv[4],Mapping::GammaToe,gamma,false,0,false,specout,toeslope);
    argc    -= 3;
    argv    += 3;
  } else if (!strcmp(arg,"--invgamma")) {
    double gamma;
    if (argc < 4)
      throw "--invgamma requires two arguments, the gamma value and a file name";
    gamma = ParseDouble(argv[2]);
    m     = new class Mapping(argv[3],Mapping::Gamma,gamma,true,0,false,specout);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--invtoegamma")) {
    double gamma,toeslope;
    if (argc < 5)
      throw "--invtoegamma requires three arguments, a toe region slope, a gamma value and a file name";
    toeslope = ParseDouble(argv[2]);
    gamma    = ParseDouble(argv[3]);
    m        = new class Mapping(argv[4],Mapping::GammaToe,gamma,true,0,false,specout,toeslope);
    argc    -= 3;
    argv    += 3;
  } else if (!strcmp(arg,"--halflog")) {
    if (argc < 3)
      throw "--halflog requires a file name argument";
    m     = new class Mapping(argv[2],Mapping::HalfLog,1.0,false,16,false,specout);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--halfexp")) {
    if (argc < 3)
      throw "--halfexp requires a file name argument";
    m     = new class Mapping(argv[2],Mapping::HalfLog,1.0,true,16,false,specout);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--togamma")) {
    long bpp;
    double gamma;
    if (argc < 4)
      throw "--togamma requires two arguments, a bit depth and a gamma value";
    bpp   = ParseLong(argv[2]);
    gamma = ParseDouble(argv[3]);
    m     = new class Mapping(NULL,Mapping::Gamma,gamma,false,bpp,true,specout);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--totoegamma")) {
    double toeslope;
    double gamma;
    if (argc < 4)
      throw "--totoegamma requires two arguments, a toe region slope and a gamma value";
    toeslope = ParseDouble(argv[2]);
    gamma    = ParseDouble(argv[3]);
    m        = new class Mapping(NULL,Mapping::GammaToe,gamma,false,0,true,specout,toeslope);
    argc    -= 2;
    argv    += 2;
  } else if (!strcmp(arg,"--fromgamma")) {
    double gamma;
    if (argc < 3)
      throw "--fromgamma requires the gamma value as argument";
    gamma = ParseDouble(argv[2]);
    m     = new class Mapping(NULL,Mapping::Gamma,gamma,true,0,true,specout);
    argc -= 1;
    argv += 1;
  } else if (!strcmp(arg,"--fromtoegamma")) {
    double toeslope;
    double gamma;
    if (argc < 3)
      throw "--fromtoegamma requires two arguments, a toe region slope and a gamma value";
    toeslope = ParseDouble(argv[2]);
    gamma    = ParseDouble(argv[3]);
    m        = new class Mapping(NULL,Mapping::GammaToe,gamma,true,0,true,specout,toeslope);
    argc    -= 2;
    argv    += 2;
  } else if (!strcmp(arg,"--tohalflog")) {
    m     = new class Mapping(NULL,Mapping::HalfLog,1.0,false,16,true,specout);
  } else if (!strcmp(arg,"--fromhalflog")) {
    m     = new class Mapping(NULL,Mapping::HalfLog,1.0,true,16,true,specout);
  } else if (!strcmp(arg,"--tolog")) {
    double clamp;
    if (argc < 3)
      throw "--tolog requires an argument, the clamp value";
    clamp = ParseDouble(argv[2]);
    m     = new class Mapping(NULL,Mapping::Log,clamp,false,32,true,specout);
    argc -= 1;
    argv += 1;
  } else if (!strcmp(arg,"--topercept")) {
    m     = new class Mapping(NULL,Mapping::PU2,0.0,false,32,true,specout);
  } else if (!strcmp(arg,"--topq")) {
    long bits;
    if (argc < 3)
      throw "--topq requires a bit depth as argument";
    bits  = ParseLong(argv[2]);
    if (bits < 8 || bits > 32)
      throw "--topq requires a bit depth between 8 and 32 bits";
    m     = new class Mapping(NULL,Mapping::PQ,1.0,false,bits,true,specout);
    argc -= 1;
    argv += 1;
  } else if (!strcmp(arg,"--frompq")) {
    m   = new class Mapping(NULL,Mapping::PQ,1.0,true, 0, true,specout);
  } else if (!strcmp(arg,"--tohlg")) {
    long bits;
    if (argc < 3)
      throw "--tohlg requires a bit depth as argument";
    bits  = ParseLong(argv[2]);
    if (bits < 8 || bits > 32)
      throw "--tohlg requires a bit depth between 8 and 32 bits";
    m     = new class Mapping(NULL,Mapping::HLG,1.0,false,bits,true,specout);
    argc -= 1;
    argv += 1;
  } else if (!strcmp(arg,"--fromhlg")) {
    m   = new class Mapping(NULL,Mapping::HLG,1.0,true, 0, true,specout);
  } else if (!strcmp(arg,"--scale")) {
    if (argc < 3)
      throw "--scale requires a comma-separated list of scaling factors";
    m     = new class WhiteBalance(WhiteBalance::Scale,argv[2]);
    argc -= 1;
    argv += 1;
  } else if (!strcmp(arg,"--offset")) {
    if (argc < 3)
      throw "--offset requires a comma-separated list of shift values";
    m     = new class WhiteBalance(WhiteBalance::Shift,argv[2]);
    argc -= 1;
    argv += 1;
  }

  return m;
}
///

/// ParseSubsampling
// Parse functions related to downsampling and upsampling
class Meter *ParseSubsampling(int &argc,char **&argv)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--sub")) {
    long sx,sy;
    if (argc < 4)
      throw "--sub requires two arguments, subsampling factors in x and y direction";
    sx = ParseLong(argv[2]);
    sy = ParseLong(argv[3]);
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "subsampling factors must be positive and smaller than 16";
    m     = new class Downsampler(sx,sy,false);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--csub")) {
    long sx,sy;
    if (argc < 4)
      throw "--sub requires two arguments, subsampling factors in x and y direction";
    sx = ParseLong(argv[2]);
    sy = ParseLong(argv[3]);
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "subsampling factors must be positive and smaller than 16";
    m     = new class Downsampler(sx,sy,true);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--up")) {
    long sx,sy;
    bool automatic = false;
    if (argc < 3)
      throw "--up requires two arguments, subsampling factors in x and y direction";
    if (!strcmp(argv[2],"auto")) {
      sx = 1;
      sy = 1;
      automatic = true;
    } else {
      if (argc < 4)
	throw "--up requires two arguments, subsampling factors in x and y direction";
      sx = ParseLong(argv[2]);
      sy = ParseLong(argv[3]);
    }
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "upsampling factors must be positive and smaller than 16";
    m     = new class Upsampler(sx,sy,false,Upsampler::Centered,automatic);
    if (automatic) {
      argc--;
      argv++;
    } else {
      argc -= 2;
      argv += 2;
    }
  } else if (!strcmp(arg,"--cup")) {
    long sx,sy;
    if (argc < 4)
      throw "--cup requires two arguments, subsampling factors in x and y direction";
    sx = ParseLong(argv[2]);
    sy = ParseLong(argv[3]);
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "upsampling factors must be positive and smaller than 16";
    m     = new class Upsampler(sx,sy,true,Upsampler::Centered);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--coup")) {
    long sx,sy;
    bool automatic = false;
    if (argc < 3)
      throw "--coup requires two arguments, subsampling factors in x and y direction";
    if (!strcmp(argv[2],"auto")) {
      sx = 1;
      sy = 1;
      automatic = true;
    } else {
      if (argc < 4)
	throw "--coup requires two arguments, subsampling factors in x and y direction";
      sx = ParseLong(argv[2]);
      sy = ParseLong(argv[3]);
    }
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "upsampling factors must be positive and smaller than 16";
    m     = new class Upsampler(sx,sy,false,Upsampler::Cosited,automatic);
    if (automatic) {
      argc--;
      argv++;
    } else {
      argc -= 2;
      argv += 2;
    }
  } else if (!strcmp(arg,"--cocup")) {
    long sx,sy;
    if (argc < 4)
      throw "--cocup requires two arguments, subsampling factors in x and y direction";
    sx = ParseLong(argv[2]);
    sy = ParseLong(argv[3]);
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "upsampling factors must be positive and smaller than 16";
    m     = new class Upsampler(sx,sy,true,Upsampler::Cosited);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--boxup")) {
    long sx,sy;
    if (argc < 4)
      throw "--boxup requires two arguments, subsampling factors in x and y direction";
    sx = ParseLong(argv[2]);
    sy = ParseLong(argv[3]);
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "upsampling factors must be positive and smaller than 16";
    m     = new class Upsampler(sx,sy,false,Upsampler::Boxed);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--boxcup")) {
    long sx,sy;
    if (argc < 4)
      throw "--boxcup requires two arguments, subsampling factors in x and y direction";
    sx = ParseLong(argv[2]);
    sy = ParseLong(argv[3]);
    if (sx >= 16 || sx <= 0 || sy >= 16 || sy <= 0)
      throw "upsampling factors must be positive and smaller than 16";
    m     = new class Upsampler(sx,sy,true,Upsampler::Boxed);
    argc -= 2;
    argv += 2;
  }

  return m;
}
///

/// ParseGeometric
// Parse various geometric manipulation functions.
class Meter *ParseGeometric(int &argc,char **&argv,const struct ImgSpecs &spec1,const struct ImgSpecs &spec2)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--flipx")) {
    m   = new class Flip(Flip::FlipX);
  } else if (!strcmp(arg,"--flipy")) {
    m   = new class Flip(Flip::FlipY);
  } else if (!strcmp(arg,"--flipxextend")) {
    m   = new class FlipExtend(FlipExtend::FlipX);
  } else if (!strcmp(arg,"--flipyextend")) {
    m   = new class FlipExtend(FlipExtend::FlipY);
  } else if (!strcmp(arg,"--shift")) {
    long dx,dy;
    if (argc < 4)
      throw "--shift requires two arguments, shift distance in horizontal and vertical direction";
    dx = ParseLong(argv[2]);
    dy = ParseLong(argv[3]);
    m     = new class Shift(dx,dy,spec1,spec2);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--crop")) {
    long x1,y1,x2,y2;
    if (argc < 2+4)
      throw "--crop requires the coordinates of the rectangle as arguments";
    x1 = ParseLong(argv[2]);
    y1 = ParseLong(argv[3]);
    x2 = ParseLong(argv[4]);
    y2 = ParseLong(argv[5]);
    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
      throw "--crop requires non-negative arguments";
    m = new class Crop(x1,y1,x2,y2,Crop::CropBoth);
    argc -= 4;
    argv += 4;
  } else if (!strcmp(arg,"--cropd")) {
    long x1,y1,x2,y2;
    if (argc < 2+4)
      throw "--cropd requires the coordinates of the rectangle as arguments";
    x1 = ParseLong(argv[2]);
    y1 = ParseLong(argv[3]);
    x2 = ParseLong(argv[4]);
    y2 = ParseLong(argv[5]);
    if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0)
      throw "--cropd requires non-negative arguments";
    m = new class Crop(x1,y1,x2,y2,Crop::CropDst);
    argc -= 4;
    argv += 4;
  } else if (!strcmp(arg,"--paste")) {
    long x1,y1;
    if (argc < 4)
      throw "--paste requires two arguments, the target x and y position";
    x1 = ParseLong(argv[2]);
    y1 = ParseLong(argv[3]);
    if (x1 < 0 || y1 < 0)
      throw "--paste requires non-negative arguments";
    m = new class Paste(x1,y1);
    argc -= 2;
    argv += 2;
  }

  return m;
}
///

/// ParseComponent
// Parse component-related manipulation functions.
class Meter *ParseComponent(int &argc,char **&argv)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
 
  if (!strcmp(arg,"--only")) {
    long comp;
    if (argc < 3)
      throw "--only requires the target component as argument";
    comp = ParseLong(argv[2]);
    if (comp < 0 || comp > MAX_UWORD)
      throw "--only argument is out of range";
    m = new class Restrict(UWORD(comp),1);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--upto")) {
    long comp;
    if (argc < 3)
      throw "--upto requires the target component as argument";
    comp = ParseLong(argv[2]);
    if (comp < 0 || comp > MAX_UWORD)
      throw "--upto argument is out of range";
    m = new class Restrict(0,comp);
    argc--;
    argv++;
  } else if (!strcmp(arg,"--rgb")) {
    m = new class Restrict(0,3);
  }

  return m;
}
///

/// ParseTotal
// Parse functions that affect one or multiple components in total.
class Meter *ParseTotal(int &argc,char **&argv)
{
  class Meter *m = NULL;
  const char *arg = argv[1];
  
  if (!strcmp(arg,"--invert")) {
    m   = new class Invert();
  } else if (!strcmp(arg,"--clamp")) {
    DOUBLE min,max;
    if (argc < 4)
      throw "--clamp requires two arguments, minimum and maximum of the valid range";
    min   = ParseDouble(argv[2]);
    max   = ParseDouble(argv[3]);
    m     = new class Clamp(min,max);
    argc -= 2;
    argv += 2;
  } else if (!strcmp(arg,"--fill")) {
    if (argc < 3)
      throw "--fill requires a comma-separated list of fill values as argument";
    m = new class Fill(argv[2]);
    argc--;
    argv++;
  }

  return m;
}
///

/// main
int main(int argc,char **argv)
{
  class Meter *agenda = NULL,*last = NULL,*m;
  const char *org = NULL;
  const char *dst = NULL;
  const char *name = argv[0];
  class ImageLayout *orgimg = NULL;
  class ImageLayout *dstimg = NULL;
  class ImageLayout *orgcpy = NULL;
  class ImageLayout *dstcpy = NULL;
  struct ImgSpecs spec1,spec2,specout;
  bool  brief = false;
  int   rc    = 0;

  try {
    while(argc > 1) {
      const char *arg = argv[1];
      //
      // No meter yet.
      m = NULL;
      if (arg[0] == '-' && arg[1] != '/') {
	if (!strcmp(arg,"--help")) {
	  Usage(argv[0]);
	  return 0;
	} else if (!strcmp(arg,"--rawhelp")) {
	  RawHelp();
	  return 0;
	} else if ((m = ParseMetrics(argc,argv))) {
	  // Done with it.
	} else if (!strcmp(arg,"--stripe")) {
	  m = new class Stripe();
	} else if ((m = ParseProperties(argc,argv))) {
	  // Done with it.
	} else if (!strcmp(arg,"--diff") || !strcmp(arg,"-i")) {
	  if (argc < 3)
	    throw "--diff requires the target file name as argument";
	  m = new class DiffImg(argv[2],specout,true);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--rawdiff") || !strcmp(arg,"-i")) {
	  if (argc < 3)
	    throw "--rawdiff requires the target file name as argument";
	  m = new class DiffImg(argv[2],specout,false);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--sdiff")) {
	  if (argc < 4)
	    throw "--sdiff requires a scale and the target file name as argument";
	  m = new class DiffImg(argv[3],specout,false,ParseDouble(argv[2]));
	  argc -= 2;
	  argv += 2;
	} else if (!strcmp(arg,"--butterfly")) {
	  if (argc < 3)
	    throw "--butterfly requires the target file name as argument";
	  m = new class Butterfly(argv[2],specout);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--suppress")) {
	  double t;
	  if (argc < 3)
	    throw "--suppress requires a threshold as argument";
	  t = ParseDouble(argv[2]);
	  if (t < 0.0)
	    throw "--suppress requires a non-negative argument";
	  m = new class Suppress(t);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--mask") || !strcmp(arg,"-R")) {
	  if (argc < 3)
	    throw "--mask requires the mask file name as argument";
	  m = new class Mask(argv[2],false);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--notmask")) {
	  if (argc < 3)
	    throw "--notmask requires the mask file name as argument";
	  m = new class Mask(argv[2],true);
	  argc--;
	  argv++;	  
	} else if (!strcmp(arg,"--convert")) {
	  if (argc < 3)
	    throw "--convert requires the target file name as argument";
	  m = new class ConvertImg(argv[2],specout);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--merge")) {
	  if (argc < 3)
	    throw "--merge requires the target file name as argument";
	  m = new class AddImg(argv[2],specout);
	  argc--;
	  argv++;
#ifdef USE_GSL
	} else if ((m = ParseFFT(argc,argv))) {
	  // done with it.
#endif
	} else if (!strcmp(arg,"--hist")) {
	  if (argc < 3)
	    throw "--hist requires a file name as argument";
	  m = new class Histogram(argv[2]);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--thres")) {
	  long t;
	  if (argc < 3)
	    throw "--thres requires a threshold as argument";
	  t = ParseLong(argv[2]);
	  if (t < 0)
	    throw "--thres requires a non-negative threshold";
	  m = new class Histogram(t);
	  argc--;
	  argv++;
	} else if (!strcmp(arg,"--colorhist")) {
	  double b;
	  if (argc < 3)
	    throw "--colorhist requires a bucket size as argument";
	  b = ParseDouble(argv[2]);
	  if (b <= 0.0)
	    throw "--colorhist requries a positive bucket size";
	  m = new class ColorHistogram(1.0 / b);
	  argc--;
	  argv++;
	} else if ((m = ParseColor(argc,argv,specout))) {
	  // done with it.
	} else if ((m = ParseBayer(argc,argv))) {
	  // done with it.
	} else if ((m = ParseConversions(argc,argv,specout))) {
	  // done with it.
	} else if ((m = ParseTransferFunctions(argc,argv,specout))) {
	  // done with it.
	} else if ((m = ParseTotal(argc,argv))) {
	  // done with it.
	} else if ((m = ParseGeometric(argc,argv,spec1,spec2))) {
	  // done with it.
	} else if ((m = ParseSubsampling(argc,argv))) {
	  // Done with it.
	} else if ((m = ParseComponent(argc,argv))) {
	  // done with it.
	} else if (!strcmp(arg,"--restore")) {
	  m = new class Restore(orgcpy,dstcpy);
	} else if (!strcmp(arg,"--raw")) {
	  specout.ASCII = ImgSpecs::No;
	} else if (!strcmp(arg,"--ascii")) {
	  specout.ASCII = ImgSpecs::Yes;
	} else if (!strcmp(arg,"--interleaved")) {
	  specout.Interleaved = ImgSpecs::Yes;
	} else if (!strcmp(arg,"--separate")) {
	  specout.Interleaved = ImgSpecs::No;
	} else if (!strcmp(arg,"--littleendian")) {
	  specout.LittleEndian = ImgSpecs::Yes;
	} else if (!strcmp(arg,"--bigendian")) {
	  specout.LittleEndian = ImgSpecs::No;
	} else if (!strcmp(arg,"--toabsradiance")) {
	  spec1.AbsoluteRadiance = ImgSpecs::Yes;
	  spec2.AbsoluteRadiance = ImgSpecs::Yes;
	} else if (!strcmp(arg,"--isyuv")) {
	  spec1.YUVEncoded = ImgSpecs::Yes;
	  spec2.YUVEncoded = ImgSpecs::Yes;
	} else if (!strcmp(arg,"--isrgb")) {
	  spec1.YUVEncoded = ImgSpecs::No;
	  spec2.YUVEncoded = ImgSpecs::No;
	} else if (!strcmp(arg,"--isfullrange")) {
	  spec1.FullRange  = ImgSpecs::Yes;
	  spec2.FullRange  = ImgSpecs::Yes;
	} else if (!strcmp(arg,"--isreducedrange")) {
	  spec1.FullRange  = ImgSpecs::No;
	  spec2.FullRange  = ImgSpecs::No;
	} else if (!strcmp(arg,"--brief")) {
	  brief = true;
	} else {
	  Usage(name);
	  throw "unknown command line option";
	}
	argv++;
	argc--;
      } else if (!strcmp(arg,">")) {
	m = new Compare(Compare::Greater,ParseDouble(argv[2]));
	argv += 2;
	argc -= 2;
      } else if (!strcmp(arg,">=")) {
	m = new Compare(Compare::GreaterEqual,ParseDouble(argv[2]));
	argv += 2;
	argc -= 2;
      } else if (!strcmp(arg,"==")) {
	m = new Compare(Compare::Equal,ParseDouble(argv[2]));
	argv += 2;
	argc -= 2;
      } else if (!strcmp(arg,"!=")) {
	m = new Compare(Compare::NotEqual,ParseDouble(argv[2]));
	argv += 2;
	argc -= 2;
      } else if (!strcmp(arg,"<=")) {
	m = new Compare(Compare::SmallerEqual,ParseDouble(argv[2]));
	argv += 2;
	argc -= 2;
      } else if (!strcmp(arg,"<")) {
	m = new Compare(Compare::Smaller,ParseDouble(argv[2]));
	argv += 2;
	argc -= 2;
      } else break;
      //
      // Created a new meter to be attached?
      if (m) {
	if (agenda == NULL) {
	  agenda = m;
	  last   = m;
	} else {
	  assert(last);
	  last->NextOf() = m;
	  last           = m;
	}
      }
    }
    if (argc == 3) {
      org = argv[1];
      dst = argv[2];
    } else {
      Usage(name);
      throw "requires exactly two mandatory arguments, original and distorted image";
    }
    if (agenda == NULL) {
      // Default: PSNR
      agenda = new class PSNR(PSNR::Mean);
    }
    assert(org && dst);
    orgimg = ImageLayout::LoadImage(org,spec1);
    if (!strcmp(dst,"-")) { 
      dstimg = ImageLayout::CloneLayout(orgimg);
    } else {
      dstimg = ImageLayout::LoadImage(dst,spec2);
    }
    // Make copies of the images.
    orgcpy   = new ImageLayout(*orgimg);
    dstcpy   = new ImageLayout(*dstimg);
    //
    specout.MergeSpecs(spec1,spec2);
    //

    //
    // Now perform the measurements on all images.
    double val = 0.0;
    for(m = agenda;m;m = m->NextOf()) {
      const char *name = m->NameOf();

      if (name) {
	// A real measurement. Compare the image dimensions.
	// Compare the images, at least the dimensions and the precisions must be
	// equal.
	orgimg->TestIfCompatible(dstimg);
      }
      
      val = m->Measure(orgimg,dstimg,val);
      if (name) {
	if (brief) {
	  printf("%g\n",val);
	} else {
	  printf("%s:\t%g\n",name,val);
	}
      }
    }
  } catch(const char *error) {
    if (org && dst)
      fprintf(stderr,"*** Program failed on %s %s : %s ***\n",org,dst,error);
    else
      fprintf(stderr,"*** Program failed : %s ***\n",error);
    rc = 10;
  } catch(const std::bad_alloc &) {
    fprintf(stderr,"*** Program run out of memory ***\n");
    rc = 15;
  } catch(...) {
    fprintf(stderr,"*** Caught unknown exception ***\n");
    rc = 20;
  }

  if (orgcpy)
    delete orgcpy;
  if (dstcpy)
    delete dstcpy;
  if (orgimg) 
    delete orgimg;
  if (dstimg)
    delete dstimg;

  while((m = agenda)) {
    agenda = m->NextOf();
    delete m;
  }

  return rc;
}
///
