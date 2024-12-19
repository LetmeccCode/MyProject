#include <graphics.h>
#include <string>
#include <vector>
#include <math.h>

// int idx_current_anim = 0;

// const int PLAYER_ANIM_NUM = 6;

// IMAGE img_player_left[PLAYER_ANIM_NUM];
// IMAGE img_player_right[PLAYER_ANIM_NUM];

// const int PLAYER_WIDTH = 80;
// const int PLAYER_HEIGHT = 80;
// const int SHADOW_WIDTH = 35;

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// POINT player_pos = {500,500};

// int PLAYER_SPEED = 3;

inline void putimage_alpha(int x,int y,IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();
    AlphaBlend(GetImageHDC(NULL),x,y,w,h,GetImageHDC(img),0,0,w,h,{AC_SRC_OVER,0,255,AC_SRC_ALPHA});
}


class Animation
{
public:
    Animation(LPCTSTR path,int num,int interval)
    {
        interval_ms = interval;

        TCHAR path_file[256];
        for(size_t i = 0;i < num;i++)
        {
            _stprintf_s(path_file,path,i);

            IMAGE* frame = new IMAGE;
            loadimage(frame,path_file);
            frame_list.push_back(frame);
        }
    }

    ~Animation()
    {
        for(size_t i =0;i < frame_list.size();i++)
        {
            delete frame_list[i];
        }
    }

    void Play(int x,int y,int delta)
    {
        timer += delta;
        if(timer >= interval_ms)
        {
            idx_frame = (idx_frame + 1) % frame_list.size();
            timer = 0;
        }

        putimage_alpha(x,y,frame_list[idx_frame]);
    }

private:
    int timer = 0;
    int idx_frame = 0;
    int interval_ms = 0;
    std::vector<IMAGE*> frame_list;
};

class Player
{
public:
    Player()
    {
        loadimage(&img_shadow,_T("resources/img/shadow_player.png"));
        anim_left = new Animation(_T("resources/img/player_left_%d.png"),6,90);
        anim_right = new Animation(_T("resources/img/player_right_%d.png"),6,90);
    }

    ~Player()
    {
        delete anim_left;
        delete anim_right;
    }

    void ProcessEvent(const ExMessage& msg)
    {

        switch (msg.message)
        {
            case WM_KEYDOWN:               
                switch (msg.vkcode)
                {
                case VK_UP:is_move_up = true;break;
                case VK_DOWN:is_move_down = true;break;
                case VK_LEFT:is_move_left = true;break;
                case VK_RIGHT:is_move_right = true;break;
                }
                break;

            case WM_KEYUP:                
                switch (msg.vkcode)
                {
                case VK_UP:is_move_up = false;break;
                case VK_DOWN:is_move_down = false;break;
                case VK_LEFT:is_move_left = false;break;
                case VK_RIGHT:is_move_right = false;break;
                }
                break;
        }
    }

    void Move_Cal()
    {
        if(is_move_up) player_pos.y -= SPEED;
        if(is_move_down) player_pos.y += SPEED;
        if(is_move_left) player_pos.x -= SPEED;
        if(is_move_right) player_pos.x += SPEED;
    }

    void Move()
    {
        int dir_x = is_move_right - is_move_left;
        int dir_y = is_move_down - is_move_up;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if(len_dir != 0)
        {   
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            player_pos.x += (int)(SPEED * normalized_x);
            player_pos.y += (int)(SPEED * normalized_y);
        }

        if(player_pos.x < 0) player_pos.x = 0;
        if(player_pos.y < 0) player_pos.y = 0;
        if(player_pos.x + FRAME_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - FRAME_WIDTH;
        if(player_pos.y + FRAME_HEIGHT > WINDOW_HEIGHT) player_pos.y = WINDOW_HEIGHT - FRAME_HEIGHT;
    }

    void Draw(int delta)
    {
        int pos_shadow_x = player_pos.x +(FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
        int pos_shadow_y = player_pos.y + FRAME_HEIGHT - 8;
        putimage_alpha(pos_shadow_x,pos_shadow_y,&img_shadow);

        static bool facing_left = false;
        int dir_x = is_move_right - is_move_left;
        if(dir_x < 0)
            facing_left = true;
        else if(dir_x > 0)
            facing_left = false;

        if(facing_left)
            anim_left->Play(player_pos.x,player_pos.y,delta);
        else
            anim_right->Play(player_pos.x,player_pos.y,delta);
    }

    POINT Getposition() const
    {
        return player_pos;
    }

private:
    const int SPEED = 3;
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;
    const int SHADOW_WIDTH = 32;
    
    IMAGE img_shadow;
    Animation* anim_left;
    Animation* anim_right;
    POINT player_pos = {500,500};
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

};

class Bullet
{
public:
    POINT position = {0 , 0};

    Bullet() = default;
    ~Bullet() = default;

    void Draw() const
    {
        setlinecolor(RGB(255,155,50));
        setfillcolor(RGB(200,75,10));
        fillcircle(position.x,position.y,RADIUS);
    }
private:
    const int RADIUS = 10;
};

class Enemy
{
public:
    Enemy()
    {
        loadimage(&img_shadow,_T("resources/img/shadow_enemy.png"));
        anim_left = new Animation(_T("resources/img/enemy_left_%d.png"),6,90);
        anim_right = new Animation(_T("resources/img/enemy_right_%d.png"),6,90);

        enum class SpawnEdge
        {
            Up = 0,
            Down,
            Left,
            Right
        };

        SpawnEdge edge = (SpawnEdge)(rand() % 4);
        switch (edge)
        {
        case SpawnEdge::Up:
            enemy_pos.x = rand() % WINDOW_WIDTH;
            enemy_pos.y = -FRAME_HEIGHT;
            break;
        case SpawnEdge::Down:
            enemy_pos.x = rand() % WINDOW_WIDTH;
            enemy_pos.y = WINDOW_HEIGHT;
            break;
        case SpawnEdge::Left:
            enemy_pos.x = -FRAME_WIDTH;
            enemy_pos.y = rand() % WINDOW_HEIGHT;
            break;
        case SpawnEdge::Right:
            enemy_pos.x = WINDOW_WIDTH;
            enemy_pos.y = rand() % WINDOW_HEIGHT;
            break;

        }
    }

    bool CheckBulletCollision(const Bullet& bullet)
    {
        return false;
    }

    bool CheckPlayerCollision(const Player& player)
    {
        return false;
    }

    void Move(const Player& player)
    {
        const POINT& player_position = player.Getposition();
        int dir_x = player_position.x - enemy_pos.x;
        int dir_y = player_position.y - enemy_pos.y;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if(len_dir != 0)
        {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            enemy_pos.x += (int)(SPEED * normalized_x);
            enemy_pos.y += (int)(SPEED * normalized_y);
        }
        if(dir_x < 0)
            facing_left = true;
        else if(dir_x > 0)
            facing_left = false;
    }

    void Draw(int delta)
    {
        int pos_shadow_x = enemy_pos.x +(FRAME_WIDTH / 2 - SHADOW_WIDTH / 2);
        int pos_shadow_y = enemy_pos.y + FRAME_HEIGHT - 35;
        putimage_alpha(pos_shadow_x,pos_shadow_y,&img_shadow);

         if(facing_left)
            anim_left->Play(enemy_pos.x,enemy_pos.y,delta);
        else
            anim_right->Play(enemy_pos.x,enemy_pos.y,delta);

    }

    ~Enemy()
    {
        delete anim_left;
        delete anim_right;
    }

private:
    const int SPEED = 2;
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;
    const int SHADOW_WIDTH = 48;
    
    IMAGE img_shadow;
    Animation* anim_left;
    Animation* anim_right;
    POINT enemy_pos = {0,0};
    bool facing_left = false;

};


// void LoadAnimation()
// {
//     for(size_t i = 0;i < PLAYER_ANIM_NUM;i++)
//     {
//         std::string path = "resources/img/player_left_" + std::to_string(i) + ".png";
//         loadimage(&img_player_left[i],(LPCTSTR)path.c_str());
//     }

//     for(size_t i = 0;i < PLAYER_ANIM_NUM;i++)
//     {
//         std::string path = "resources/img/player_right_" + std::to_string(i) + ".png";
//         loadimage(&img_player_right[i],(LPCTSTR)path.c_str());
//     }
// }

// Animation anim_left_player(_T("resources/img/player_left_%d.png"),6,90);
// Animation anim_right_player(_T("resources/img/player_right_%d.png"),6,90);
//    Player Pai_men;

// IMAGE img_shadow;

// void DrawPlayer(int delta,int dir_x)
// {
//     int pos_shadow_x = player_pos.x +(PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
//     int pos_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
//     putimage_alpha(pos_shadow_x,pos_shadow_y,&img_shadow);

//     static bool facing_left = false;
//     if(dir_x < 0)
//         facing_left = true;
//     else if(dir_x > 0)
//         facing_left = false;

//     if(facing_left)
//         anim_left_player.Play(player_pos.x,player_pos.y,delta);
//     else
//         anim_right_player.Play(player_pos.x,player_pos.y,delta);
// } 

void TryGenerateEnemy(std::vector<Enemy*>& enemy_list)
{
    const int INTERVAL = 100;
    static int counter = 0;
    if((++counter) % INTERVAL == 0)
        enemy_list.push_back(new Enemy);
}

int main()
{  
    initgraph(1280,720);

    bool running = true;

    ExMessage msg;
    IMAGE img_background;

    Player Pai_men;
    std::vector<Enemy*> enemy_list;

    // bool is_move_up = false;
    // bool is_move_down = false;
    // bool is_move_left = false;
    // bool is_move_right = false;

    // LoadAnimation();

    //loadimage(&img_shadow,_T("resources/img/shadow_player.png"));
    loadimage(&img_background,_T("resources/img/background.png"));
    
    BeginBatchDraw();

    while(running)
    {
        DWORD start_time = GetTickCount();

        while(peekmessage(&msg))
        {
            // if(msg.message == WM_KEYDOWN)
            // {
            //     switch (msg.vkcode)
            //     {
            //     case VK_UP:is_move_up = true;break;
            //     case VK_DOWN:is_move_down = true;break;
            //     case VK_LEFT:is_move_left = true;break;
            //     case VK_RIGHT:is_move_right = true;break;
            //     }
              
            // }
            // else if(msg.message == WM_KEYUP)
            // {
            //     switch (msg.vkcode)
            //     {
            //     case VK_UP:is_move_up = false;break;
            //     case VK_DOWN:is_move_down = false;break;
            //     case VK_LEFT:is_move_left = false;break;
            //     case VK_RIGHT:is_move_right = false;break;
            //     }
            // }
            Pai_men.ProcessEvent(msg);
        }
        // if(is_move_up) player_pos.y -= PLAYER_SPEED;
        // if(is_move_down) player_pos.y += PLAYER_SPEED;
        // if(is_move_left) player_pos.x -= PLAYER_SPEED;
        // if(is_move_right) player_pos.x += PLAYER_SPEED;
        Pai_men.Move_Cal();
        Pai_men.Move();
        TryGenerateEnemy(enemy_list);
        for(Enemy* enemy : enemy_list)
            enemy->Move(Pai_men);
        // int dir_x = is_move_right - is_move_left;
        // int dir_y = is_move_down - is_move_up;
        // double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        // if(len_dir != 0)
        // {   
        //     double normalized_x = dir_x / len_dir;
        //     double normalized_y = dir_y / len_dir;
        //     player_pos.x += (int)(PLAYER_SPEED * normalized_x);
        //     player_pos.y += (int)(PLAYER_SPEED * normalized_y);
        // }

        // if(player_pos.x < 0) player_pos.x = 0;
        // if(player_pos.y < 0) player_pos.y = 0;
        // if(player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
        // if(player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT) player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT;

        // static int counter = 0;
        // if(++counter % 5 == 0)
        //     idx_current_anim++;

        // idx_current_anim = idx_current_anim % PLAYER_ANIM_NUM;

        cleardevice();

        putimage(0,0,&img_background);
        Pai_men.Draw(1000 / 60);
        for(Enemy* enemy : enemy_list)
            enemy->Draw(1000 / 60);
        // DrawPlayer(1000 / 60,is_move_right - is_move_left);


        FlushBatchDraw();

        DWORD end_time = GetTickCount();
        DWORD delta_time = end_time - start_time;
        if(delta_time < 1000 / 60)
        {
            Sleep(1000 / 60 - delta_time);
        }
    }

    EndBatchDraw();


    return 0;
}