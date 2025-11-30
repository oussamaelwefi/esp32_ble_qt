import onnxruntime
import numpy as np
import pandas as pd

# --- Configuration ---
ONNX_FILENAME = "health_classifier.onnx"

# The exact feature names used in your conversion script
FEATURE_NAMES = [
    'gender',
    'gestational_age_weeks',
    'birth_weight_kg',
    'birth_length_cm',
    'age_days',
    'weight_kg',
    'length_cm',
    'temperature_c',
    'heart_rate_bpm'
]

# --- 1. Create Sample Input Data ---
# IMPORTANT: This sample data should be realistic and match the scale
# of the data your model was trained on.
sample_data = {
    'gender': ['Female'],       # String input
    'gestational_age_weeks': [38.5], # Float input
    'birth_weight_kg': [3.1],
    'birth_length_cm': [50.0],
    'age_days': [100.0],
    'weight_kg': [5.5],
    'length_cm': [60.0],
    'temperature_c': [37.0],
    'heart_rate_bpm': [140.0]
}

df_sample = pd.DataFrame(sample_data, columns=FEATURE_NAMES)

# --- 2. Prepare Data for ONNX Runtime ---
# ONNX runtime expects a dictionary mapping the input name to a NumPy array.
# For models with ColumnTransformer, each feature is its own input tensor.

onnx_inputs = {}
for name in FEATURE_NAMES:
    data = df_sample[name].values
    
    # Reshape to (num_samples, 1) to match the [None, 1] shape in initial_type
    reshaped_data = data.reshape(-1, 1) 
    
    if name == 'gender':
        # String data needs to be passed as an object array of strings
        onnx_inputs[name] = np.array(reshaped_data, dtype=object)
    else:
        # Float data needs to be passed as float32, which is standard for ONNX
        onnx_inputs[name] = reshaped_data.astype(np.float32)


# --- 3. Load Model and Predict ---
try:
    # Create an inference session
    sess = onnxruntime.InferenceSession(ONNX_FILENAME)
    
    # Get the names of the outputs the model produces (usually 'output_label' and 'output_probability')
    output_names = [output.name for output in sess.get_outputs()]
    
    print(f"--- Running Prediction on {ONNX_FILENAME} ---")
    print(f"Sample Input: {df_sample.iloc[0].to_dict()}")
    
    # Run the model!
    onnx_results = sess.run(output_names, onnx_inputs)
    
    # Extract the results (assuming the model outputs a label and probabilities)
    predicted_label = onnx_results[0]
    probabilities = onnx_results[1] if len(onnx_results) > 1 else None # Probability is optional

    print("\n✅ Prediction Successful!")
    print(f"Predicted Label (Class): {predicted_label[0]}")
    if probabilities is not None:
        print(f"Output Probabilities: {probabilities[0]}")
    
except Exception as e:
    print(f"\n❌ An error occurred during ONNX inference: {e}")
    print("Please verify your ONNX file or the input preparation step.")


# --- 4. Validation Check (Optional but highly recommended) ---
# To fully validate, you should load the original joblib model and run the 
# same sample data through it to ensure the results match exactly.
# This requires you to prepare the data exactly as the original model expects (e.g., as a single DataFrame 
# or a pre-processed array) and run: scikit_model.predict(df_sample)
