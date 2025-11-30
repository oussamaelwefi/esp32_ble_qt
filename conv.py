from skl2onnx import convert_sklearn
from skl2onnx.common.data_types import FloatTensorType, StringTensorType
import joblib

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

# Load the trained model from the file created in Step 1
MODEL_FILENAME = 'my_health_classifier.joblib'
scikit_model = joblib.load(MODEL_FILENAME)

# --- NEW STEP: Extract the Best Estimator from RandomizedSearchCV ---
# The error shows the original object is a RandomizedSearchCV.
# We must convert the final trained model (the best_estimator_) instead.
final_estimator = scikit_model.best_estimator_

# --- CRITICAL: Define the model's input structure ---
# The number '9' is kept based on your original code's initial_type.
# Ensure '9' is the exact number of features (columns) your model takes.
initial_type = []
for name in FEATURE_NAMES:
    if name == 'gender':
        # Categorical features must be defined as StringTensorType for OrdinalEncoder
        initial_type.append((name, StringTensorType([None, 1])))
    else:
        # All other features are numerical (Float)
        initial_type.append((name, FloatTensorType([None, 1])))
# Note: Using 'None' instead of '1' for the first dimension makes the model 
# compatible with batches of any size, which is a common best practice.

# Convert the best scikit-learn model object into the ONNX format
onx = convert_sklearn(
    # Use the extracted final_estimator here
    final_estimator,
    initial_types=initial_type,
    target_opset=14, # Using a common ONNX operator set
    # The 'options' may need adjustment depending on the actual estimator type
    # For a Decision Tree/Pipeline, this option might not be strictly needed,
    # but we will apply it to the type of the *best_estimator* for safety.
    options={type(final_estimator): {"zipmap": False}}
)

# Save the ONNX model to the final file
ONNX_FILENAME = "health_classifier.onnx"
with open(ONNX_FILENAME, "wb") as f:
    f.write(onx.SerializeToString())

print(f"ONNX model successfully converted and saved as: {ONNX_FILENAME}")

# You may also see the InconsistentVersionWarning, which is a minor issue
# but should ideally be fixed by training the model and running this conversion
# in the same environment (same scikit-learn version 1.7.1).
