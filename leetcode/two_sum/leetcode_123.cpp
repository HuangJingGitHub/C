// will exceed the time limit.
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        if (prices.size() < 2)
            return 0;
        
        int res = 0, currentLowest = prices[0];
        for(int i = 1; i < prices.size(); i++){
            if (prices[i] <= currentLowest){
                currentLowest = prices[i];
                continue;
            }
            res = max(res, maxProfitDay(prices, i));
            currentLowest = prices[i];
        }
        return res;
    }

    int maxProfitDay(vector<int>& prices, int sellDay){
        //if (sellDay < 0 || sellDay >= prices.size())
        //    return 0;

        int res1 = 0, res2 = 0, lowestProfit1 = prices[0];
        for (int i = 1; i <= sellDay; i++){
            res1 = max(res1, prices[i] - lowestProfit1);
            lowestProfit1 = min(lowestProfit1, prices[i]);
        }

        if (sellDay >= prices.size()-2)
            return res1;
        
        int lowestProfit2 = prices[sellDay+1];
        for (int i = sellDay+2; i < prices.size(); i++){
            res2 = max(res2, prices[i] - lowestProfit2);
            lowestProfit2 = min(lowestProfit2, prices[i]);
        }
        return res1 + res2;
    }
};

// state machine method, very cool
class Solution {
public:
    int maxProfit(vector<int>& prices) {
        if (prices.size() < 2)
            return 0;
        int buy1 = -prices[0], sell1 = 0, buy2 = -prices[0], sell2 = 0;  // represpent the moneny after ith purchase or sale

        for (int i = 1; i < prices.size(); i++){
            buy1 = max(buy1, -prices[i]);
            sell1 = max(sell1, buy1 + prices[i]);
            buy2 = max(buy2, sell1 - prices[i]);
            sell2 = max(sell2, buy2 + prices[i]);
        }
        return max(0, sell2);
    }
};
