class Solution {
public:
    string shortestPalindrome(string s) {
        string res;
        int len = s.size(), mid = 0;
        
        while (len < 2 * s.size()) {
            mid = len / 2 - (len - s.size());
            if (len % 2 == 0) {
                if (checkPalindrome(s, mid - 1, mid))
                    break;
            }
            else {
                if (checkPalindrome(s, mid, mid))
                    break;
            }
            len++;
        }

        res = s.substr(2 * s.size() - len);
        reverse(res.begin(), res.end());
        res += s;
        return res;
    }
    
    bool checkPalindrome(string& s, int left, int right) {
        while (left >= 0) {
            if (s[left] != s[right])
                return false;
            left--;
            right++;
        }
        return true;
    }
};


// KMP algorithm 1
class Solution {
public:
    int computeNext(string pattern) {
        vector<int> next(pattern.size() + 1, 0);
        next[0] = -1;
        next[1] = 0;
        int i = 2, k = 0;
        while (i < next.size()) {
            if (pattern[i - 1] == pattern[k]) {
                next[i] = k + 1;
                k = next[i];
                i++;
            }
            else if (k == 0)
                i++;
            else 
                k = next[k];
        }
        return next.back();
    }

    string shortestPalindrome(string s) {
        if (s.size() <= 1)
            return s;
        string sRev(s.rbegin(), s.rend()), pattern = s + "#" + sRev;
        int maxLen = computeNext(pattern);
        return sRev.substr(0, sRev.size() - maxLen) + s;
    }
};


// KMP algorithm 2
class Solution {
public:
    int computeNext(string pattern) {
        vector<int> next(pattern.size() + 1, 0);
        next[0] = -1;
        int pos = 1, cnt = 0;
        while (pos < pattern.size()) {
            if (pattern[pos] == pattern[cnt]) {
                next[pos] = next[cnt];
            }
            else {
                next[pos] = cnt;
                while (cnt >= 0 && pattern[pos] != pattern[cnt])
                    cnt = next[cnt];
            }
            pos++;
            cnt++;
        }
        next[pos] = cnt;
        return next.back();
    }

    string shortestPalindrome(string s) {
        if (s.size() <= 1)
            return s;
        string sRev(s.rbegin(), s.rend()), pattern = s + "#" + sRev;
        int maxLen = computeNext(pattern);
        return sRev.substr(0, sRev.size() - maxLen) + s;
    }
};
