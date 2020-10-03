// BFS, speed is slow for large scale input.
class Solution {
public:
    int ladderLength(string beginWord, string endWord, vector<string>& wordList) {
        unordered_set<string> wordSet(wordList.begin(), wordList.end());
        if (wordSet.find(endWord) == wordSet.end())
            return 0;

        int res = 1;
        unordered_set<string> visited;
        queue<vector<string>> pathQueue;
        vector<string> path{beginWord};
        pathQueue.push(path);
        visited.emplace(beginWord);

        while (!pathQueue.empty()){
            int queueSize = pathQueue.size();
            unordered_set<string> subVisited;

            for (int i = 0; i < queueSize; i++){
                path = pathQueue.front();
                pathQueue.pop();
                string curEndWord = path.back();
                for (int j = 0; j < curEndWord.size(); j++){
                    for (char ch = 'a'; ch <= 'z'; ch++){
                        string newWord = curEndWord;
                        newWord.replace(j, 1, 1, ch);
                        if (newWord  == curEndWord)
                            continue;
                        if (wordSet.find(newWord) != wordSet.end() && visited.find(newWord) == visited.end()){
                            if (newWord == endWord)
                                return path.size() + 1;

                            vector<string> pathVec = path;
                            pathVec.push_back(newWord);
                            pathQueue.push(pathVec);
                            subVisited.emplace(newWord);
                        }            
                    }
                }
            }
            visited.insert(subVisited.begin(), subVisited.end());
        }
        return 0;
    }
};


class Solution {
public:
    int ladderLength(string beginWord, string endWord, vector<string>& wordList) {
        if (std::find(wordList.begin(), wordList.end(), endWord) == wordList.end() )
            return 0;

        vector<int> visited(wordList.size(), 0);
        queue<vector<string>> pathQueue;
        vector<string> path{beginWord};
        pathQueue.push(path);

        while (!pathQueue.empty()){
            int queueSize = pathQueue.size();
            vector<int> subVisitedIdx;

            for (int i = 0; i < queueSize; i++){
                path = pathQueue.front();
                pathQueue.pop();
                string curEndWord = path.back();
                
                for (int i = 0; i < wordList.size(); i++){
                    if (visited[i] == 1)
                        continue;

                    if (checkTrans(curEndWord, wordList, i)){
                        if (wordList[i] == endWord)
                            return path.size() + 1;

                        path.push_back(wordList[i]);
                        pathQueue.push(path);
                        subVisitedIdx.push_back(i);
                    }

                }
            }
            for (int i : subVisitedIdx)
                visited[i] = 1;
        }
        return 0;
    }

    bool checkTrans(string &curWord, vector<string> &wordList, int wordIdx){
        int diffCount = 0;
        for (int i = 0; i < curWord.size(); i++){
            if (curWord[i] != wordList[wordIdx][i])
                diffCount++;
        }

        return diffCount == 1;
    }
};
