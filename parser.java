import java.util.function.Function;
import java.util.regex.*;
import java.util.*;
import java.io.BufferedReader;
import java.io.*;
import java.util.ArrayList;

public class Parser
   {
   public static final int opcodeToInt(String opCode)
      {
      switch (opCode)
         {
         case "PUSH": return 1;
         case "+" : return 2;
         case "-" : return 3;
         case "*" : return 4;
         case "/" : return 5;
         
         case "==": return 6;
         case "!=": return 7;
         case ">" : return 8;
         case "<" : return 9;
         case ">=" : return 10;
         case "<=" : return 11;
         
         case "&" : return 12;
         case "|" : return 13;
         case "^" : return 14;
         case "NOT": return 15;
         case "NEGATE": return 16;
         case "AS_CHAR": return 17;
         
         case "MODULO" : return 18;
         case "SHL" : return 19;
         case "SHR" : return 20;
         
         default:
         throw new IllegalArgumentException("Unknown op code: " + opCode);
         }
      }
   public static String replaceEach(String regex, 
   Function<MatchResult, String> replaceFunction, String toReplaceIn)
      {
      Pattern pattern = Pattern.compile(regex);
      Matcher matcher = pattern.matcher(toReplaceIn);
      ArrayList<String> matches = new ArrayList<>();
      
      StringBuilder replacer = new StringBuilder(toReplaceIn);
      int[] shiftBy = new int[]{0};

      matcher.results().forEach((match) -> {
         String replacement = replaceFunction.apply(match);
         replacer.replace(match.start() + shiftBy[0], match.end() + shiftBy[0], replacement);
            
         shiftBy[0] -= (match.end() - match.start()) - replacement.length();
         });
      return replacer.toString();
      }
   public static ArrayList<String> readFile(File file) throws Exception
      {
      ArrayList<String> lines = new ArrayList<>();
      
      BufferedReader buf = new BufferedReader(new FileReader(file));
      String content;
      while((content = buf.readLine()) != null)
         {
         lines.add(content);
         }
      return lines;
      }
      
   public static String[] opcode2Precedence = new String[]{
      "|", "^", "&", "==", "!=", "<<", ">>", "<", "<=", ">", ">=", "+", "-", "*", "/", "%"
      };
   public static ArrayList<Integer> parse(ArrayList<String> lines)
      {
      ArrayList<Integer> instructionStack = new ArrayList<>();
      
      String line = lines.get(0);
      
      for(int i = 0; i < opcode2Precedence.length; i++)
         {
         line = replaceEach(String.format("(-?[\\d]+)[ ]*(\\%s)[ ]*(-?[\\d]+)", opcode2Precedence[i]), (result) -> {
            instructionStack.add(1);
            instructionStack.add(Integer.parseInt(result.group(1)));
            instructionStack.add(1);
            instructionStack.add(Integer.parseInt(result.group(3)));
            instructionStack.add(opcodeToInt(result.group(2)));
            return "";
            }, line);
         }
         
      System.out.println(instructionStack); 
      return instructionStack;
      }
   public static void main(String[] args)
      {
      try{
         if(args.length < 1)
            {
            System.out.println("Expected at least 1 filename argument");
            }
         else
            {
            parse(readFile(new File(args[0])));
            }
         }
      catch(Exception e)
         {
         System.out.println("Unable to read file");
         e.printStackTrace();
         }
      }
   }