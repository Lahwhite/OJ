package org.example.users.utils;

public class CredentialValidationUtils {
    public static boolean isNumber(char ch) {
        return (ch >= '0' && ch <= '9');
    }
    public static boolean isUnderscore(char ch) {
        return ch == '_';
    }
    public static boolean isUpperCase(char ch) {
        return ch >= 'A' && ch <= 'Z';
    }
    public static boolean isLowerCase(char ch) {
        return ch >= 'a' && ch <= 'z';
    }
    public static boolean isAlphabet(char ch) {
        return isUpperCase(ch) || isLowerCase(ch);
    }
    public static boolean isLegal(char ch) {
        return ch >= '!' && ch <= '~';
    }

    public static boolean checkPassword(String password) {
        if (password == null) {
            return false;
        }
        if (password.length() < 8 || password.length() > 64) {
            return false;
        }

        int upper = 0;
        int lower = 0;
        int number = 0;
        int special = 0;
        char ch;
        for (int i = 0; i < password.length(); i++) {
            ch = password.charAt(i);
            if (!isLegal(ch)) {
                return false;
            }
            if (isUpperCase(ch)) {
                upper += 1;
            }
            else if (isLowerCase(ch)) {
                lower += 1;
            }
            else if (isNumber(ch)) {
                number += 1;
            }
            else  {
                special += 1;
            }
        }

        return upper != 0 && lower != 0 && number != 0 && special != 0;
    }

    public static boolean checkUsername(String username) {
        if (username == null) {
            return false;
        }
        if (username.length() < 3 || username.length() > 32) {
            return false;
        }

        char ch = username.charAt(0);
        if (!isAlphabet(ch)) {
            return false;
        }
        for (int i = 1; i < username.length(); i++) {
            ch = username.charAt(i);
            if (!isAlphabet(ch) && !isNumber(ch) && !isUnderscore(ch)) {
                return false;
            }
        }
        return true;
    }
}