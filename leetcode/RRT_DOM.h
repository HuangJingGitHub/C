#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <eigen3/Eigen/Dense>
#include <eigen3/Eigen/Eigenvalues>

using namespace cv;
using namespace std;

struct RRT_Node {
    Point2f pos;
    RRT_Node* parent;
    vector<RRT_Node*> adjacency_list;
    RRT_Node(): pos(Point2f(0, 0)), parent(nullptr) {}
    RRT_Node(Point2f initPos): pos(initPos), parent(nullptr) {}
};

class RRT_Planner {
public:
    Point2f start_pos_;
    Point2f target_pos_;
    float step_len_;
    float error_dis_;
    Size2f config_size_;
    RRT_Node* search_graph_;
    RRT_Node* graph_end_;
    int MAX_GRAPH_SIZE = 5000;
    int CUR_GRAPH_SIZE = 0;
    bool plan_scuess_ = false;


    RRT_Planner(): search_graph_(nullptr), graph_end_(nullptr) {}
    RRT_Planner(Point2f start, Point2f target, float step_len = 10, float error_dis = 10,
                Size2f config_size = Size2f(640, 480)): 
        start_pos_(start), 
        target_pos_(target), 
        step_len_(step_len), 
        error_dis_(error_dis),
        config_size_(config_size) {
        search_graph_ = new RRT_Node(start);
        graph_end_ = new RRT_Node();
        CUR_GRAPH_SIZE++;
    }
    ~RRT_Planner();
    RRT_Planner(const RRT_Planner&);
    RRT_Planner& operator=(const RRT_Planner&);

    bool Plan(Mat source_img) {
        srand(time(NULL));
        plan_scuess_ = false;
        float div_width = RAND_MAX / config_size_.width,
              div_height = RAND_MAX / config_size_.height,
              min_dis_to_target = 1e4;
        Point2f rand_pos = Point2f(0, 0);
        
        while (CUR_GRAPH_SIZE < MAX_GRAPH_SIZE) {
            rand_pos.x = rand() / div_width;
            rand_pos.y = rand() / div_height;

            cout << "*****-->CUR_GRAPH_SIZE " << CUR_GRAPH_SIZE << '\n';
            RRT_Node* nearest_node = NearestNode(rand_pos);
            // cout << "Nearest_node address: " << nearest_node << " --> pos "<< nearest_node->pos << '\n';
            RRT_Node* new_node = AddNewNode(nearest_node, rand_pos);
            cout << "Added new node address: " << new_node << " --> pos " << new_node->pos << '\n';
            if (norm(new_node->pos - target_pos_) <= error_dis_) {
                cout << "Find path!!!\n";
                graph_end_= new_node;
                plan_scuess_ = true;
                return true;
            }
            circle(source_img, start_pos_, 5, Scalar(0,0,255), -1);
            circle(source_img, target_pos_, 5, Scalar(0,0,255), -1);
            circle(source_img, new_node->pos, 3, Scalar(0,255,0), -1);
            imshow("RRT path planning", source_img);
            waitKey(10);
            // float temp_dis = norm(new_node->pos - target_pos_);
            // min_dis_to_target = min(min_dis_to_target, temp_dis);
            // cout << "Current min distance to target: " << min_dis_to_target << '\n';
            CUR_GRAPH_SIZE++;
        }
        cout << "MAX_GRAPH_SIZE: " << MAX_GRAPH_SIZE << " is achieved with no path founded.\n";
        return false;
    }

    RRT_Node* NearestNode(Point2f& rand_node) {
        RRT_Node* res = search_graph_;
        queue<RRT_Node*> level_pt;
        float min_dis = norm(rand_node - search_graph_->pos), cur_dis;
        // cout << "Pushed node pointer:\n";
        for (auto pt : search_graph_->adjacency_list) {
            level_pt.push(pt);
            // cout << pt << '\n';
        }
        
        // bfs
        // cout << "search_graph_: " << search_graph_ << '\n';
        while (!level_pt.empty()) {
            queue<RRT_Node*> tempQueue = level_pt;
            // cout << "RRT_Node queue size: " << tempQueue.size() << "\n";
            //while(!tempQueue.empty()) {
            //    if (!tempQueue.front()->adjacency_list.empty())
            //        cout << tempQueue.front() << "\n";
            //    tempQueue.pop();
            //}

            int level_size = level_pt.size();
            for (int i = 0; i < level_size; i++) {
                RRT_Node* cur_node = level_pt.front();
                level_pt.pop();

                //string s;
                //cin >> s;

                // cout << "Current Node Position: " << cur_node->pos << '\n';
                // cout << "Current Node adjacency_list: \n";
                for (auto pt : cur_node->adjacency_list) {
                    level_pt.push(pt);
                    // cout << pt << ' ';
                }
                cur_dis = norm(rand_node - cur_node->pos);
                // cout << "\nCurrent distance " << cur_dis << '\n';
                if (cur_dis < min_dis)
                    res = cur_node;
            }
        }
        return res;
    }

    RRT_Node* AddNewNode(RRT_Node* nearest_node, Point2f& rand_pos) {
        Point2f direction = (rand_pos - nearest_node->pos) / norm((rand_pos - nearest_node->pos));
        Point2f new_pos = nearest_node->pos + direction * step_len_;
        //RRT_Node new_node_obj = RRT_Node(new_pos);
        //RRT_Node* new_node = &new_node_obj;
        RRT_Node* new_node = new RRT_Node(new_pos);
        // cout << "New node address: " << new_node << '\n';
        new_node->parent = nearest_node;
        nearest_node->adjacency_list.push_back(new_node);
        return new_node;
    }

    vector<RRT_Node*> GetPath() {
        vector<RRT_Node*> res;
        if (!plan_scuess_) {
            cout << "No valid path available.\n";
            return res;
        }
        RRT_Node* reverse_node = graph_end_;
        while (reverse_node) {
            res.push_back(reverse_node);
            reverse_node = reverse_node->parent;
        }
        reverse(res.begin(), res.end());
        return res;   
    }    
};


RRT_Planner::~RRT_Planner() {
    delete search_graph_;
    delete graph_end_;
}

RRT_Planner::RRT_Planner(const RRT_Planner& planner) {
    start_pos_ = planner.start_pos_;
    target_pos_ = planner.target_pos_;
    step_len_ = planner.step_len_;
    error_dis_ = planner.error_dis_;
    config_size_ = planner.config_size_;
    if (!search_graph_)
        search_graph_ = new RRT_Node((*planner.search_graph_));
    else
        *search_graph_ = *(planner.search_graph_);
    if (!graph_end_)
        graph_end_ = new RRT_Node(*(planner.graph_end_));
    else
        *graph_end_ = *(planner.graph_end_);
    MAX_GRAPH_SIZE = planner.MAX_GRAPH_SIZE;
    CUR_GRAPH_SIZE = planner.CUR_GRAPH_SIZE;
    plan_scuess_ = planner.plan_scuess_;        
}

RRT_Planner& RRT_Planner::operator=(const RRT_Planner& rhs) {
    start_pos_ = rhs.start_pos_;
    target_pos_ = rhs.target_pos_;
    step_len_ = rhs.step_len_;
    error_dis_ = rhs.error_dis_;
    config_size_ = rhs.config_size_;
    if (!search_graph_)
        search_graph_ = new RRT_Node((*rhs.search_graph_));
    else
        *search_graph_ = *(rhs.search_graph_);
    if (!graph_end_)
        graph_end_ = new RRT_Node(*(rhs.graph_end_));
    else
        *graph_end_ = *(rhs.graph_end_);
    MAX_GRAPH_SIZE = rhs.MAX_GRAPH_SIZE;
    CUR_GRAPH_SIZE = rhs.CUR_GRAPH_SIZE;
    plan_scuess_ = rhs.plan_scuess_;
}
