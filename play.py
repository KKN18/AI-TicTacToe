# -*- coding: utf-8 -*-
"""Play.ipynb

Automatically generated by Colaboratory.

Original file is located at
    https://colab.research.google.com/drive/1RsNcXmOcorbqhIioPM4lecktrDYgF8KW
"""

# !pip install gym

# Commented out IPython magic to ensure Python compatibility.
# !git clone https://github.com/ClementRomac/gym-tictactoe
# %cd gym-tictactoe
# !python setup.py install
import argparse
import gym
import gym_tictactoe
import random
import numpy as np
env = gym.make('TicTacToe-v1', symbols=[-1, 1], board_size=3, win_size=3)

import tensorflow as tf
from tensorflow.keras import Sequential, layers
from tensorflow.keras.layers import Dense
from tensorflow.keras.optimizers import Adam
import torch

if not torch.cuda.is_available():
    print("WARNING: You have to run with CUDA device")

parser = argparse.ArgumentParser()
parser.add_argument('--user_mode', type=int, default=0, help='First player : 0, Second player : 1')
parser.add_argument('--ckpt0_path', type=str, default='/content/drive/MyDrive/Colab Notebooks/Reinforcement Learning/us0_weight/', help='directory of user0 weight')
parser.add_argument('--ckpt1_path', type=str, default='/content/drive/MyDrive/Colab Notebooks/Reinforcement Learning/us1_weight/', help='directory of user1 weight')
opt = parser.parse_args()

def build_model():
    model = Sequential()
    model.add(Dense(128, input_dim=9, activation='relu'))
    model.add(Dense(52, activation='relu'))
    model.add(Dense(9, activation='softmax'))
    model.compile(loss='mse', optimizer=Adam())
    return model

us0_model = build_model()

us0_weight_dir = opt.ckpt0_path + 'us0.ckpt'
us0_model.load_weights(us0_weight_dir)

us1_model = build_model()

us1_weight_dir = opt.ckpt1_path +'us1.ckpt'
us1_model.load_weights(us1_weight_dir)

def us0_predictor(s):
    return np.argmax(us0_model.predict(np.array(s).reshape(-1, 9))[0])

def us1_predictor(s):
    return np.argmax(us1_model.predict(np.array(s).reshape(-1, 9))[0])

def extra_us0_predictor(s, n):
    lst = us0_model.predict(np.array(s).reshape(-1, 9))[0]
    for i in range(n-1):
        iMax = np.argmax(lst)
        lst[iMax] = -1
    return np.argmax(lst)
def extra_us1_predictor(s, n):
    lst = us1_model.predict(np.array(s).reshape(-1, 9))[0]
    for i in range(n-1):
        iMax = np.argmax(lst)
        lst[iMax] = -1
    return np.argmax(lst)

def us0_play_with_AI():
    user = 0
    done = False
    reward = 0
    render = True
    # Reset the env before playing
    state = env.reset()

    while not done:
        env.render(mode=None)
        if user == 0:
            usr_action = int(input("입력 : "))
            while 0 > usr_action or usr_action > 8 or state[usr_action] != 0:
                print("잘못 입력하셨습니다.")
                usr_action = int(input("입력 : "))
            state, reward, done, infos = env.step(usr_action, -1)
        elif user == 1:
            nTry = 2
            us1_action = us1_predictor(state)
            while state[us1_action] != 0:
                us1_action = extra_us1_predictor(state, nTry)
                nTry += 1
            state, reward, done, infos = env.step(us1_action, 1)
          
        # If the game isn't over, change the current player
        if not done:
            user = 0 if user == 1 else 1
        else :
            if reward == 10:
                print("Draw !")
            elif reward == -20:
                print("Infos : " + str(infos))
                if user == 0:
                    print("AI wins!")
                elif user == 1:
                    print("User wins!")
            elif reward == 20:
                if user == 0:
                    print("User wins!")
                elif user == 1:
                    print("AI wins!")
    env.render(mode=None)


def us1_play_with_AI():
    user = 0
    done = False
    reward = 0
    render = True
    # Reset the env before playing
    state = env.reset()

    while not done:
        env.render(mode=None)
        if user == 0:
            nTry = 2
            us0_action = us0_predictor(state)
            while state[us0_action] != 0:
                us0_action = extra_us0_predictor(state, nTry)
                nTry += 1
            state, reward, done, infos = env.step(us0_action, -1)
        elif user == 1:
            usr_action = int(input("입력 : "))
            while 0 > usr_action or usr_action > 8 or state[usr_action] != 0:
                print("잘못 입력하셨습니다.")
                usr_action = int(input("입력 : "))
            state, reward, done, infos = env.step(usr_action, 1)

        # If the game isn't over, change the current player
        if not done:
            user = 0 if user == 1 else 1
        else :
            if reward == 10:
                print("Draw !")
            elif reward == -20:
                print("Infos : " + str(infos))
                if user == 0:
                    print("User wins!")
                elif user == 1:
                    print("AI wins!")
            elif reward == 20:
                if user == 0:
                    print("AI wins!")
                elif user == 1:
                    print("User wins!")
    env.render(mode=None)

if opt.user_mode == 0:
    us0_play_with_AI()
else:
    us1_play_with_AI()

