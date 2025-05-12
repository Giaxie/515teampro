import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from sklearn.metrics import classification_report, confusion_matrix
import seaborn as sns
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Conv1D, MaxPooling1D, LSTM, Dense, Dropout
from tensorflow.keras.callbacks import EarlyStopping

# Configuration
DATA_PATH = "/Users/laura/Desktop/515teampro/data"  # Change this path as needed
CLASS_NAMES = ["C", "N", "S"]  # Circular, Nodding, Swiping
LABEL_MAP = {name: idx for idx, name in enumerate(CLASS_NAMES)}
SAMPLE_LENGTH = 101  # 1 second at 100Hz
NUM_FEATURES = 3

# Load and preprocess data
def load_data():
    X, y = [], []
    for class_name in CLASS_NAMES:
        folder = os.path.join(DATA_PATH, class_name)
        for file in os.listdir(folder):
            if file.endswith(".csv"):
                df = pd.read_csv(os.path.join(folder, file))
                if df.shape[0] == SAMPLE_LENGTH:
                    data = df[["x", "y", "z"]].values
                    X.append(data)
                    y.append(LABEL_MAP[class_name])
    return np.array(X), np.array(y)

print("Loading data...")
X, y = load_data()
print(f"Loaded {X.shape[0]} samples with shape {X.shape[1:]}.")

# Normalize per sample
X = (X - X.mean(axis=1, keepdims=True)) / (X.std(axis=1, keepdims=True) + 1e-6)

# Train/test split
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, stratify=y, random_state=42)

# Build CNN + LSTM model
model = Sequential([
    Conv1D(32, kernel_size=3, activation='relu', input_shape=(SAMPLE_LENGTH, NUM_FEATURES)),
    MaxPooling1D(pool_size=2),
    Dropout(0.3),
    LSTM(64),
    Dense(32, activation='relu'),
    Dense(len(CLASS_NAMES), activation='softmax')
])

model.compile(optimizer='adam', loss='sparse_categorical_crossentropy', metrics=['accuracy'])
model.summary()

# Train
early_stop = EarlyStopping(monitor='val_loss', patience=5, restore_best_weights=True)
model.fit(X_train, y_train, epochs=30, batch_size=16, validation_split=0.2, callbacks=[early_stop])

# Evaluate
y_pred = np.argmax(model.predict(X_test), axis=1)
print("\nClassification Report:")
print(classification_report(y_test, y_pred, target_names=CLASS_NAMES))

# Confusion matrix
cm = confusion_matrix(y_test, y_pred)
sns.heatmap(cm, annot=True, fmt='d', xticklabels=CLASS_NAMES, yticklabels=CLASS_NAMES, cmap='Blues')
plt.xlabel("Predicted")
plt.ylabel("True")
plt.title("Confusion Matrix")
plt.tight_layout()
plt.show()

# Save model
model.save("motion_model_cnn_lstm.h5")
print("Model saved as motion_model_cnn_lstm.h5")
