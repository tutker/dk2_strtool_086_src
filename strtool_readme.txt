Dungeon Keeper 2 STR text strings tool
------------------------------

Program description:

 This tool allows to export and import texts from STR files
  in Dungeon Keeper 2. These files can then be used in maps
  to display text messages for a player.

 You can export any STR file into unicode text file (.TXT),
  then modify it using any text editor (ie. Notepad), then
  import it again into STR file.

 To learn how to use these texts in your maps, open one of the
  original campaign maps and see how it's done there. In short,
  you have to type the STR file name in Map Editor, and use
  line numbers to display specific message. Detailed tutorial
  is also available in this document.

 Program's source code files comes with Dev-C++ project files,
  so you can easily rebuild the source using Dev-C++ IDE or GCC.

Editing STR files:

 When editing the text file, do not change the number in first
  line - it is required to create STR file properly; its exact
  meaning is not known, but in texts for standard maps, it is
  incremented by one in every file.
  Also, the "\" is a special character for the converter, so if
  you want to use "\" in your message, type "\\" in the text file.

 Be sure not to insert new lines between existing texts! Texts are
  identified by their line numbers, so by inserting new lines
  you're destroying the indexing. If you want to add new messages,
  replace the 'Placefiller' slots.

 Files "Speech.str" and "Text.str" have one more feature:
  parameters. Into some of the messages, the game pastes numbers,
  names of creatures etc. In the text file, parameters are marked
  by the "%" (percentage) sign, like "%1" or "%24". To display the
  actual percentage sign in the message, you must place "%%" in
  text file - single "%" will be treated as parameter, not as
  a text to display.

 To create a new STR file, use export function on a file where no
  text slots are used (like DEMO1), and then rename the exported
  text file. When creating STR file, the destination file name is
  always identical to the '.TXT' file name, so by renaming the text
  file, you're also changing name of the '.STR'.

 Only "strtool.exe" and "MBToUni.dat" form original DK2 distribution
  is required for the program to work. Please note that "MBToUni.dat"
  file is different for various language versions; you must use
  a version for your language. You will find the correct "MBToUni.dat"
  in the "Data\Text" subfolder where you've installed DK2.

Adding text messages to map with Official DK2 Editor:

 It's easier to replace PLACEHOLDER spaces in existing STRs than to
 create new STRs. So let's say you felt like adding onto "level1.str".
 You just need to use STRTool to convert the STR file to text,
 add your new lines in Notepad, then convert it back to STR.

 In your level, you just need to trigger the text now. First, open
 "Level" - "Edit Level Variables", and set 'Text Table ID' to 'Level 1'.
 Optionally, you can set 'Sound Category' to SPEECH_LEVEL1, so the
 default audio plays. That's it, you're done here.

 Under the script tree, create a trigger. Any trigger you want.
 But for example, let's say you want your new text to play at the start
 of the level. So 'Add Trigger' to your player (Keeper 1 by default)
 and select the 'Attribute' tab.
 Now set it to "When THIS LEVEL's TIME is GREATER/EQUAL to 0" and hit OK.

 Now to create the action. 'Add Action' to the new trigger and select the
 Info tab. You have two options there. Play Speech or Display Text String.

 "Display Text String" option does not work properly, so select "Play Speech".
 Set the ID number to the line where you placed your new text when you edited
 the TXT file. If you use Notepad, turn Word Wrap off, and count the lines.
 Make sure the 'Show Text' box is checked, and hit OK.

 That's it, you're done. The new text you created will show.
 If you used Play Sound, Your advisor will likely say "Placeholder"
 when the text comes up if you set the Sound Category earlier.
 The SDT audio files can be edited too, adding your own sounds
 under Placeholders.

Creating new STR files:
 
 If you don't want to modify any of official files, but to create a new one,
 the task may become problematic. This is because in official editor you can't
 select any value from 'Text Table ID' - you may only choose one of official
 ones.

 It is surely possible to store the text messages and briefing texts in other
 files, but a mechanism of doing it isn't documented yet. If you will succeed
 in adding messages from new STR files to a level, please write a little
 tutorial so I could add it to this documentation.

Installation:

 Copy the 'strtool.exe' into your 'DK2\Data\Text\Default' folder.
 You may also use other folder, but then you will have to type
  the path to your DK2 folder every time you run it.

 If you don't want to modify your original DK2 files, you may place
  this program in any folder on your hard drive and copy the content
  of 'Default' folder from DK2 into that place.

 All files used by this program are originally placed in the
  'Default' folder, so this ends the process of instalation.

Usage:
  strtool <strfile> <operation>
Valid <operations> are:
  x: eXport entries into text file
  c: Create the str file using text file
  d: Dump str file structure data

Example 1 (extract level1.str into text file level1.txt):
  strtool level1 x

Example 2 (create secret1.str using text file secret1.txt):
  strtool secret1 c

Version: 0.8.6
 Tutorial added to documentation
 Source code commentary fixed

Version: 0.8.5
 Fixed Unicode support in Windows issue
 Added icon

Version: 0.8.0b
 Parameters (like %1) now supported
 Full support of all STR files now ready
 Removed parts of old and unused code
 Disabled debug dump on processing
 Added short and simple information messages

Version: 0.7.0b
 STR entries decoder rewritten
 Corrected handling of '\x' entries
 Prepared to make support of parameters (unfinished)

Version: 0.6.0b
 Creating STRs support rewritten
 All supported Unicode chars are now properly coded
 Parameters (%) not supported yet by both decoder and encoder

Version: 0.5.0b
 Creating STRs supported
 Only some characters are properly coded

Version: 0.4.5b
 Export to text file working on most files
 Added support of '\\', '\r' and '\n'.
 Started making support of STR chunk 0x02

Version: 0.4.0b
 Made export to text files

Version: 0.3.0b
 First version; info dumping only

Author:
 Tomasz Lis, listom@gmail.com

Credits:
 Thanks to Nebster and Trass3r for their description of STR files
 which saved me a lot of time.
 Thanks to Solar for the tutorial on how to add custom text
 messages to map.
 
