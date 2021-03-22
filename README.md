# AI-TicTacToe
## Environment
모든 작업이 Google Colab Notebooks에서 진행되었습니다.

[OpenAI 틱택토 게임 환경](https://github.com/ClementRomac/gym-tictactoe)을 이용하였습니다.

## Setup
 먼저, 틱택토 게임환경을 불러옵니다.

    git clone https://github.com/ClementRomac/gym-tictactoe
    cd gym-tictactoe
    python setup.py install
    
 해당 repository를 불러오고 다음 커맨드들을 입력합니다.

    git clone https://github.com/KKN18/AI-TicTacToe.git
    cd AI-TicTacToe
    !pip install -q pyyaml h5py
   
 원하는 epoch만큼 강화학습 모델을 학습하고 us0_weight, us1_weight 폴더에 각각 선공, 후공 모델 가중치를 저장합니다.
 
    python tictactoe.py --epochs 10 --ckpt0_path '/content/drive/MyDrive/Colab Notebooks/GitHub/AI-TicTacToe/us0_weight' --ckpt1_path '/content/drive/MyDrive/Colab Notebooks/GitHub/AI-TicTacToe/us1_weight'

 학습이 끝나면 생성된 AI와 틱택토 게임을 진행할 수 있습니다. (user_mode 0 : 선공, 1 : 후공)
 
    python play.py --user_mode 1 --ckpt0_path '/content/drive/MyDrive/Colab Notebooks/GitHub/AI-TicTacToe/us0_weight' --ckpt1_path '/content/drive/MyDrive/Colab Notebooks/GitHub/AI-TicTacToe/us1_weight'
    
## Play Result
### < us0_play_with_AI >

<img src = "https://github.com/KKN18/AI-TicTacToe/blob/main/result/us0_result.PNG">

### < us1_play_with_AI >

<img src = "https://github.com/KKN18/AI-TicTacToe/blob/main/result/us1_result.PNG">

## < Shortcoming >
<img src = "https://github.com/KKN18/AI-TicTacToe/blob/main/result/shortcome.PNG">

"자신이 이기는 방법"은 학습했지만 "상대가 이기는 걸 막는 방법"은 학습하지 못한 것으로 보입니다.

이를 위한 해결책을 고안중입니다.

## Reference
[Gym TicTacToe](https://github.com/ClementRomac/gym-tictactoe)

[OpenAI Gym 튜토리얼](http://www.secmem.org/blog/2019/03/09/OpenAI-Gym-%EC%82%AC%EC%9A%A9%ED%95%98%EA%B8%B0/)
