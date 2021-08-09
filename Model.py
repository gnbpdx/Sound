import torch
import torch.utils.data
import numpy as np
from Spectogram import Spectogram
import os
import sys
class Model:

    def __init__(self, model, trainloader, testloader, optimizer, device, criterion, num_inputs):
        self.model = model
        self.trainloader = trainloader
        self.testloader = testloader
        self.optimizer = optimizer
        self.device = device
        self.criterion = criterion
        self.num_inputs = num_inputs
        self.model = self.model.to(device)

    def train(self, cnn : bool = False):
        total_loss = 0
        for _, batch in enumerate(self.trainloader, 0):

            inputs, labels = batch
            inputs = inputs.to(self.device)
            labels = labels.to(self.device)

            self.optimizer.zero_grad()
            output = None
            if cnn:
                inputs = inputs.reshape(len(labels), 1, 150,150)
                output = self.model(inputs.float())
                output = output.view(output.size(0), -1)
            else:
                output = self.model(inputs.float(), self.num_inputs)
            loss = self.criterion(output, labels)
            total_loss += loss.item()
            loss.backward()
            torch.nn.utils.clip_grad_norm_(self.model.parameters(), 1)
            self.optimizer.step()
        return total_loss / len(self.trainloader)

    def test(self, cnn = False):
        total = 0
        correct = 0
        with torch.no_grad():
            for batch in self.testloader:
                inputs, labels = batch
                inputs = inputs.to(self.device)
                labels = labels.to(self.device)
                if cnn:
                    inputs = inputs.reshape(len(labels), 1, 150,150)
                    output = self.model(inputs.float())
                    output = output.view(output.size(0), -1)
                else:
                    output = self.model(inputs.float(), self.num_inputs)
                output = output.to('cpu')
                labels = labels.to('cpu')
                for i in range(len(inputs)):
                    prediction = int(torch.argmax(output[i]))
                    true_label = int(labels[i])
                    if prediction == true_label:
                        correct += 1
                    total += 1
        return correct / total

class Spectogram_DataLoader(torch.utils.data.Dataset):
    def __init__(self, spectogram_files):
        self.length = len(spectogram_files)
        self.spectograms = []
        for spectogram_file, label in spectogram_files:
            print("Loading " + spectogram_file)
            spectogram = Spectogram.load_spectogram_from_file(spectogram_file)
            #Arguably spectogram in db scale is a better image 
            spectogram = Spectogram.real_spectogram_to_db_spectogram(spectogram)
            spectogram = torch.from_numpy(spectogram)
            self.spectograms.append((spectogram, label))
    
    def __len__(self):
        return self.length

    def __getitem__(self, idx):
        return self.spectograms[idx]
        
class FC(torch.nn.Module):
    def __init__(self, activation, num_hidden_nodes, num_inputs, outputs):
        super().__init__()
        num_hidden_nodes.insert(0, num_inputs)
        num_hidden_nodes.append(outputs)
        self.layers = torch.nn.ModuleList()
        for i in range(len(num_hidden_nodes) - 1):
            self.layers.append(torch.nn.Linear(num_hidden_nodes[i], num_hidden_nodes[i+1]))
        self.activation = activation

    def forward(self, x, num_inputs):
        x= x.view(-1, num_inputs)
        for layer in self.layers[:-1]:
            x = self.activation(layer(x))
        x = self.layers[-1](x)
        return x

def dataset(data_type):
    files = []
    prefix = None
    if data_type == 'train':
        prefix = 'training_data/'

    elif data_type == 'test':
        prefix = 'test_data/'

    else:
        return None

    label_dirs = os.listdir(prefix)
    for label_dir in label_dirs:
        if 'spec' not in label_dir:
            continue
        if '4' not in label_dir and '7' not in label_dir:
            continue
        path = prefix + label_dir + '/'
        for spectogram_file in os.listdir(path):
            files.append((path + spectogram_file, 0 if '4' in label_dir else 1))

    return Spectogram_DataLoader(files)

        
def train_model():
    trainset = dataset('train')
    testset = dataset('test')
    trainloader = torch.utils.data.DataLoader(trainset, batch_size = 32, shuffle = True)
    testloader = torch.utils.data.DataLoader(testset, batch_size = 32, shuffle = False)
    fc_model = FC(torch.relu, [500, 500,500], 150*150, 2)
    optimizer = torch.optim.SGD(fc_model.parameters(), lr = 0.0001, momentum = 0.0001)
    device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
    criterion = torch.nn.CrossEntropyLoss()
    model = Model(fc_model, trainloader, testloader, optimizer, device, criterion, 150*150)
    for epoch in range(10000):
        print('Epoch ' + str(epoch))
        print(model.train(cnn=False))
        print(model.test(cnn=False))
        torch.save(fc_model.state_dict(), 'model')

def eval_model():
    testset = dataset('test')
    testloader = torch.utils.data.DataLoader(testset, batch_size = 32, shuffle = False)
    fc_model = FC(torch.relu, [500, 500,500], 150*150, 2)
    fc_model.load_state_dict(torch.load('model'))
    fc_model.eval()
    device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
    model = Model(fc_model, None, testloader, None, device, None, 150*150)
    print("Accuracy: " + str(model.test()))

def classify_wav_file(wav_files):
    fc_model = FC(torch.relu, [500, 500,500], 150*150, 2)
    fc_model.load_state_dict(torch.load('model'))
    fc_model.eval()
    device = torch.device('cuda:0' if torch.cuda.is_available() else 'cpu')
    fc_model = fc_model.to(device)
    spec = Spectogram()
    for wav_file in wav_files:
        spectogram = spec.signal_to_spectogram(wav_file)
        spectogram = Spectogram.compress_spectogram(np.abs(spectogram), (150,150))
        spectogram = Spectogram.real_spectogram_to_db_spectogram(spectogram)
        spectogram = torch.from_numpy(spectogram)
        spectogram = spectogram.to(device)
        output = fc_model(spectogram.float(), 150*150)
        prediction = int(torch.argmax(output))
        if prediction == 0:
            print(wav_file + ': Piano')
        elif prediction == 1:
            print(wav_file + ': Violin')
        else:
            assert False


if __name__ == '__main__':
    classify_wav_file(sys.argv[1:])

