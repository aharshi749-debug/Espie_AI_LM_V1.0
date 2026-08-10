#MAKE SURE BEFORE RUNNING THIS, MAKE SURE TO DO pip install torch torchvision torchaudio --index-url https://pytorch.org    FOR CPU TRAINING
#IF YOU DONT WANT CPU, DO THIS ONE, pip install torch

import random
import string
import numpy as np
import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import IterableDataset, DataLoader

VOCAB_SIZE = 256
EMBED_DIM = 64
CONTEXT_LEN = 256
HIDDEN_DIM = 96

EPOCHS = 10
BATCH_SIZE = 256
BATCHES_PER_EPOCH = 800
LEARNING_RATE = 0.0005
WEIGHT_DECAY = 1e-5

FLASH_MODEL_ADDR = 0x3A0000
OUTPUT_FILE = "espie_1_6m_sgm.bin"

TOP_K = 16
TEMPERATURE = 1.0

SEED = 42

EMBED_OFFSET = 0
FC1_W_OFFSET = VOCAB_SIZE * EMBED_DIM
FC1_B_OFFSET = FC1_W_OFFSET + (
HIDDEN_DIM * (CONTEXT_LEN * EMBED_DIM)
)
FC2_W_OFFSET = FC1_B_OFFSET + HIDDEN_DIM
FC2_B_OFFSET = FC2_W_OFFSET + (
VOCAB_SIZE * HIDDEN_DIM
)

EXPECTED_SIZE = (
VOCAB_SIZE * EMBED_DIM
+ HIDDEN_DIM * (CONTEXT_LEN * EMBED_DIM)
+ HIDDEN_DIM
+ VOCAB_SIZE * HIDDEN_DIM
+ VOCAB_SIZE
)

def set_seed(seed):
random.seed(seed)
np.random.seed(seed)
torch.manual_seed(seed)

```
if torch.cuda.is_available():
    torch.cuda.manual_seed_all(seed)
```

class StraightThroughRound(torch.autograd.Function):

```
@staticmethod
def forward(ctx, value):
    return torch.round(value)

@staticmethod
def backward(ctx, grad_output):
    return grad_output
```

def fake_int8(value):
value = torch.clamp(value, -127.0, 127.0)
return StraightThroughRound.apply(value)

def fake_ternary(value):
value = torch.clamp(value, -1.0, 1.0)
return StraightThroughRound.apply(value)

class DeploymentLinear(nn.Module):

```
def __init__(self, in_features, out_features):
    super().__init__()

    self.weight = nn.Parameter(
        torch.empty(
            out_features,
            in_features
        )
    )

    self.bias = nn.Parameter(
        torch.empty(out_features)
    )

    nn.init.uniform_(
        self.weight,
        -1.0,
        1.0
    )

    nn.init.uniform_(
        self.bias,
        -1.0,
        1.0
    )

def forward(self, x):
    weight = fake_ternary(
        self.weight
    )

    bias = fake_int8(
        self.bias
    )

    return torch.nn.functional.linear(
        x,
        weight,
        bias
    )
```

class EspieModel(nn.Module):

```
def __init__(self):
    super().__init__()

    self.embed = nn.Embedding(
        VOCAB_SIZE,
        EMBED_DIM
    )

    self.fc1 = DeploymentLinear(
        CONTEXT_LEN * EMBED_DIM,
        HIDDEN_DIM
    )

    self.fc2 = DeploymentLinear(
        HIDDEN_DIM,
        VOCAB_SIZE
    )

    nn.init.uniform_(
        self.embed.weight,
        -16.0,
        16.0
    )

def forward(self, x):

    embedding = fake_int8(
        self.embed(x)
    )

    flat = embedding.reshape(
        x.size(0),
        CONTEXT_LEN * EMBED_DIM
    )

    hidden = self.fc1(flat)

    hidden = torch.relu(
        hidden
    )

    logits = self.fc2(
        hidden
    )

    return logits
```

NAMES = [
"Alex",
"Sam",
"Jordan",
"Taylor",
"Morgan",
"Casey",
"Riley",
"Jamie",
"Avery",
"Drew"
]

ANIMALS = [
"dog",
"cat",
"bird",
"horse",
"rabbit",
"fish"
]

OBJECTS = [
"book",
"computer",
"phone",
"car",
"tree",
"house",
"robot"
]

ADJECTIVES = [
"small",
"large",
"bright",
"quiet",
"fast",
"old",
"young",
"green",
"blue"
]

VERBS = [
"runs",
"jumps",
"moves",
"works",
"waits",
"reads",
"sleeps"
]

LOCATIONS = [
"Atlanta",
"London",
"Tokyo",
"Paris",
"Berlin",
"Boston"
]

COLORS = [
"red",
"blue",
"green",
"yellow",
"black",
"white"
]

def random_word():
length = random.randint(
3,
9
)

```
return "".join(
    random.choices(
        string.ascii_lowercase,
        k=length
    )
)
```

def make_example():

```
task = random.randrange(
    24
)

if task == 0:
    a = random.randint(
        0,
        999
    )
    b = random.randint(
        0,
        999
    )

    return (
        f"add {a} {b}",
        str(a + b)
    )

if task == 1:
    a = random.randint(
        0,
        999
    )
    b = random.randint(
        0,
        999
    )

    return (
        f"subtract {a} {b}",
        str(a - b)
    )

if task == 2:
    a = random.randint(
        0,
        99
    )
    b = random.randint(
        0,
        99
    )

    return (
        f"multiply {a} {b}",
        str(a * b)
    )

if task == 3:
    a = random.randint(
        0,
        999
    )

    return (
        f"double {a}",
        str(a * 2)
    )

if task == 4:
    a = random.randint(
        0,
        50
    )

    return (
        f"square {a}",
        str(a * a)
    )

if task == 5:
    word = random_word()

    return (
        f"reverse {word}",
        word[::-1]
    )

if task == 6:
    word = random_word()

    return (
        f"first letter of {word}",
        word[0]
    )

if task == 7:
    word = random_word()

    return (
        f"last letter of {word}",
        word[-1]
    )

if task == 8:
    name = random.choice(
        NAMES
    )

    return (
        f"my name is {name}",
        f"Hello {name}."
    )

if task == 9:
    name = random.choice(
        NAMES
    )

    return (
        f"what is my name {name}",
        f"Your name is {name}."
    )

if task == 10:
    location = random.choice(
        LOCATIONS
    )

    return (
        f"I live in {location}",
        f"That is in {location}."
    )

if task == 11:
    location = random.choice(
        LOCATIONS
    )

    return (
        f"where do I live {location}",
        f"You live in {location}."
    )

if task == 12:
    noun = random.choice(
        ANIMALS
    )

    adjective = random.choice(
        ADJECTIVES
    )

    verb = random.choice(
        VERBS
    )

    sentence = (
        f"The {adjective} "
        f"{noun} {verb}."
    )

    return (
        f"identify the noun in {sentence}",
        f"The noun is {noun}."
    )

if task == 13:
    noun = random.choice(
        ANIMALS
    )

    adjective = random.choice(
        ADJECTIVES
    )

    verb = random.choice(
        VERBS
    )

    sentence = (
        f"The {adjective} "
        f"{noun} {verb}."
    )

    return (
        f"identify the adjective in {sentence}",
        f"The adjective is {adjective}."
    )

if task == 14:
    noun = random.choice(
        ANIMALS
    )

    verb = random.choice(
        VERBS
    )

    return (
        f"what is a {noun} doing",
        f"The {noun} is {verb}."
    )

if task == 15:
    color = random.choice(
        COLORS
    )

    obj = random.choice(
        OBJECTS
    )

    return (
        f"what color is the {obj}",
        f"The {obj} can be {color}."
    )

if task == 16:
    noun = random.choice(
        ANIMALS
    )

    return (
        f"what is a {noun}",
        f"A {noun} is an animal."
    )

if task == 17:
    obj = random.choice(
        OBJECTS
    )

    return (
        f"what is a {obj}",
        f"A {obj} is a thing."
    )

if task == 18:
    return (
        "does ice melt",
        "Yes, ice melts when heated."
    )

if task == 19:
    return (
        "is the sun hot",
        "Yes, the sun is very hot."
    )

if task == 20:
    return (
        "what do plants need",
        "Plants need water, light, and nutrients."
    )

if task == 21:
    return (
        "does gravity pull objects down",
        "Yes, gravity pulls objects downward."
    )

if task == 22:
    word = random_word()

    return (
        f"spell {word}",
        word
    )

word = random_word()

return (
    f"say the word {word}",
    word
)
```

def make_context(prompt, answer_prefix):

```
combined = (
    prompt
    + answer_prefix
)

encoded = combined.encode(
    "ascii",
    errors="replace"
)

if len(encoded) >= CONTEXT_LEN:
    return np.frombuffer(
        encoded[-CONTEXT_LEN:],
        dtype=np.uint8
    ).copy()

result = np.full(
    CONTEXT_LEN,
    ord(" "),
    dtype=np.uint8
)

result[
    CONTEXT_LEN - len(encoded):
] = np.frombuffer(
    encoded,
    dtype=np.uint8
)

return result
```

def make_training_sample():

```
prompt, answer = make_example()

answer = answer[:80]

position = random.randint(
    0,
    len(answer) - 1
)

prefix = answer[:position]

context = make_context(
    prompt,
    prefix
)

target = ord(
    answer[position]
)

return (
    torch.from_numpy(
        context.astype(
            np.int64
        )
    ),
    torch.tensor(
        target,
        dtype=torch.long
    )
)
```

class GeneratedDataset(
IterableDataset
):

```
def __init__(
    self,
    samples
):
    self.samples = samples

def __iter__(self):

    for _ in range(
        self.samples
    ):
        yield make_training_sample()
```

@torch.no_grad()
def generate_python(
model,
prompt,
max_length=80
):

```
model.eval()

generated = ""

for _ in range(
    max_length
):

    context = make_context(
        prompt,
        generated
    )

    x = torch.from_numpy(
        context.astype(
            np.int64
        )
    ).unsqueeze(0)

    logits = model(
        x
    )[0]

    values, indices = torch.topk(
        logits,
        TOP_K
    )

    values = values / TEMPERATURE

    probabilities = torch.softmax(
        values,
        dim=0
    )

    choice = torch.multinomial(
        probabilities,
        1
    )

    token = int(
        indices[
            choice
        ].item()
    )

    if token == 10:
        break

    if token < 32 or token > 126:
        continue

    generated += chr(
        token
    )

return generated
```

VALIDATION = [
(
"add 613 208",
"821"
),
(
"subtract 907 44",
"863"
),
(
"multiply 37 81",
"2997"
),
(
"double 417",
"834"
),
(
"square 31",
"961"
),
(
"reverse planet",
"tenalp"
),
(
"first letter of computer",
"c"
),
(
"last letter of computer",
"r"
),
(
"what is a dog",
"A dog is an animal."
),
(
"does ice melt",
"Yes, ice melts when heated."
)
]

@torch.no_grad()
def evaluate(
model,
device
):

```
model.eval()

exact = 0
total = len(
    VALIDATION
)

for prompt, expected in VALIDATION:

    generated = generate_python(
        model,
        prompt
    )

    generated = generated.strip()

    if generated == expected:
        exact += 1

    print(
        f"[TEST] {prompt}"
    )

    print(
        f"       Expected: {expected}"
    )

    print(
        f"       Generated: {generated}"
    )

return (
    exact / max(
        1,
        total
    )
)
```

def export_binary(model):

```
model.eval()
model.cpu()

with torch.no_grad():

    embed = torch.clamp(
        torch.round(
            model.embed.weight
        ),
        -127,
        127
    ).to(torch.int8)

    fc1_weight = torch.clamp(
        torch.round(
            model.fc1.weight
        ),
        -1,
        1
    ).to(torch.int8)

    fc1_bias = torch.clamp(
        torch.round(
            model.fc1.bias
        ),
        -127,
        127
    ).to(torch.int8)

    fc2_weight = torch.clamp(
        torch.round(
            model.fc2.weight
        ),
        -1,
        1
    ).to(torch.int8)

    fc2_bias = torch.clamp(
        torch.round(
            model.fc2.bias
        ),
        -127,
        127
    ).to(torch.int8)

parts = [
    embed,
    fc1_weight,
    fc1_bias,
    fc2_weight,
    fc2_bias
]

sizes = [
    part.numel()
    for part in parts
]

actual_size = sum(
    sizes
)

if actual_size != EXPECTED_SIZE:
    raise RuntimeError(
        "Binary size mismatch: "
        f"expected {EXPECTED_SIZE}, "
        f"got {actual_size}"
    )

with open(
    OUTPUT_FILE,
    "wb"
) as file:

    for part in parts:
        file.write(
            part.numpy().tobytes()
        )

print(
    f"Binary size: "
    f"{actual_size:,} bytes"
)

print(
    f"Binary size: "
    f"{actual_size / 1024 / 1024:.3f} MiB"
)

print(
    f"Output: {OUTPUT_FILE}"
)

print(
    f"Flash address: "
    f"{hex(FLASH_MODEL_ADDR)}"
)
```

def inspect_weights(model):

```
with torch.no_grad():

    embed = torch.clamp(
        torch.round(
            model.embed.weight
        ),
        -127,
        127
    )

    fc1 = torch.clamp(
        torch.round(
            model.fc1.weight
        ),
        -1,
        1
    )

    fc2 = torch.clamp(
        torch.round(
            model.fc2.weight
        ),
        -1,
        1
    )

    fc1_bias = torch.clamp(
        torch.round(
            model.fc1.bias
        ),
        -127,
        127
    )

    fc2_bias = torch.clamp(
        torch.round(
            model.fc2.bias
        ),
        -127,
        127
    )

print(
    "Embedding range:",
    int(embed.min()),
    int(embed.max())
)

print(
    "FC1 ternary:",
    {
        -1: int(
            (fc1 == -1).sum()
        ),
        0: int(
            (fc1 == 0).sum()
        ),
        1: int(
            (fc1 == 1).sum()
        )
    }
)

print(
    "FC2 ternary:",
    {
        -1: int(
            (fc2 == -1).sum()
        ),
        0: int(
            (fc2 == 0).sum()
        ),
        1: int(
            (fc2 == 1).sum()
        )
    }
)

print(
    "FC1 bias range:",
    int(fc1_bias.min()),
    int(fc1_bias.max())
)

print(
    "FC2 bias range:",
    int(fc2_bias.min()),
    int(fc2_bias.max())
)
```

def main():

```
set_seed(
    SEED
)

print(
    "[1/4] Booting Espie SGM"
)

device = torch.device(
    "cuda"
    if torch.cuda.is_available()
    else "cpu"
)

print(
    f"Device: {device}"
)

model = EspieModel().to(
    device
)

print(
    f"Expected binary size: "
    f"{EXPECTED_SIZE:,} bytes"
)

print(
    f"Expected binary size: "
    f"{EXPECTED_SIZE / 1024 / 1024:.3f} MiB"
)

print(
    "[2/4] Creating generated training stream"
)

dataset = GeneratedDataset(
    BATCHES_PER_EPOCH
    * BATCH_SIZE
)

dataloader = DataLoader(
    dataset,
    batch_size=BATCH_SIZE,
    num_workers=0,
    pin_memory=torch.cuda.is_available()
)

criterion = nn.CrossEntropyLoss()

optimizer = optim.AdamW(
    model.parameters(),
    lr=LEARNING_RATE,
    weight_decay=WEIGHT_DECAY
)

scheduler = optim.lr_scheduler.ReduceLROnPlateau(
    optimizer,
    mode="min",
    patience=1,
    factor=0.5
)

print(
    "[3/4] Training"
)

for epoch in range(
    EPOCHS
):

    model.train()

    total_loss = 0.0
    batches = 0

    for batch_x, batch_y in dataloader:

        batch_x = batch_x.to(
            device,
            non_blocking=True
        )

        batch_y = batch_y.to(
            device,
            non_blocking=True
        )

        optimizer.zero_grad(
            set_to_none=True
        )

        logits = model(
            batch_x
        )

        loss = criterion(
            logits,
            batch_y
        )

        if not torch.isfinite(
            loss
        ):
            raise RuntimeError(
                "Non-finite loss detected"
            )

        loss.backward()

        torch.nn.utils.clip_grad_norm_(
            model.parameters(),
            max_norm=1.0
        )

        optimizer.step()

        total_loss += loss.item()
        batches += 1

        if batches % 100 == 0:

            print(
                f"Epoch {epoch + 1}/{EPOCHS} "
                f"Batch {batches}/{BATCHES_PER_EPOCH} "
                f"Loss {loss.item():.4f}"
            )

    average_loss = (
        total_loss
        / max(
            1,
            batches
        )
    )

    scheduler.step(
        average_loss
    )

    print(
        f"Epoch {epoch + 1} complete "
        f"Loss {average_loss:.4f} "
        f"LR "
        f"{optimizer.param_groups[0]['lr']:.7f}"
    )

    accuracy = evaluate(
        model,
        device
    )

    print(
        f"Generation exact-match: "
        f"{accuracy * 100:.2f}%"
    )

print(
    "[4/4] Inspecting and exporting"
)

inspect_weights(
    model
)

export_binary(
    model
)

print()
print(
    "Flash command:"
)

print(
    f"esptool.py --port /dev/ttyUSB0 "
    f"write_flash "
    f"{hex(FLASH_MODEL_ADDR)} "
    f"{OUTPUT_FILE}"
)

print()
print(
    "Done."
)
```

if **name** == "**main**":
main()

