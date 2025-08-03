using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class DoorController : MonoBehaviour
{
    [Header("Door Animation Settings")]
    public float openAngle = 90f;
    public float openSpeed = 2f;
    public bool slideInsteadOfRotate = false;
    public Vector3 slideDirection = Vector3.right;
    public float slideDistance = 3f;

    [Header("Force Requirements")]
    public float minimumForceRequired = 5f;
    public bool requiresBallHit = true;

    [Header("Detection Settings")]
    public float detectionRadius = 5f; // How close the ball needs to be to trigger
    public LayerMask ballLayer = -1; // What layers count as the ball

    [Header("Door State")]
    public bool isOpen = false;
    public bool canClose = true;
    public float closeDelay = 5f;

    [Header("Audio & Effects")]
    public AudioSource openSound;
    public AudioSource closeSound;
    public ParticleSystem hitEffect;

    private Vector3 originalPosition;
    private Quaternion originalRotation;
    private bool isAnimating = false;
    private Coroutine closeCoroutine;
    private SphereCollider detectionSphere;

    void Start()
    {
        originalPosition = transform.position;
        originalRotation = transform.rotation;

        // Create a larger invisible sphere collider for better detection
        CreateDetectionSphere();

        // Ensure door has proper collider setup
        Collider col = GetComponent<Collider>();
        if (col != null)
        {
            col.isTrigger = false;
        }

        Debug.Log($"Door Controller initialized. Detection radius: {detectionRadius}, Min force: {minimumForceRequired}");
    }

    void CreateDetectionSphere()
    {
        // Create a child object with a large sphere collider for detection
        GameObject detectionObj = new GameObject("BallDetection");
        detectionObj.transform.SetParent(transform);
        detectionObj.transform.localPosition = Vector3.zero;

        detectionSphere = detectionObj.AddComponent<SphereCollider>();
        detectionSphere.radius = detectionRadius;
        detectionSphere.isTrigger = true;

        // Add the detection script
        BallDetectionTrigger detectionScript = detectionObj.AddComponent<BallDetectionTrigger>();
        detectionScript.doorController = this;
    }

    // Called by ThrowableBall script when ball hits door
    public void OnBallHit(float impactForce, Vector3 hitPoint)
    {
        Debug.Log($"Door received ball hit with force: {impactForce}");

        // Create hit effect if available
        if (hitEffect != null)
        {
            hitEffect.transform.position = hitPoint;
            hitEffect.Play();
        }

        // Check if force is sufficient to open door
        if (impactForce >= minimumForceRequired)
        {
            if (!isOpen && !isAnimating)
            {
                Debug.Log("Force sufficient - Opening door!");
                StartCoroutine(OpenDoor());
            }
            else if (isOpen && canClose && !isAnimating)
            {
                Debug.Log("Door already open - Closing door!");
                StartCoroutine(CloseDoor());
            }
        }
        else
        {
            Debug.Log($"Force too weak. Need at least {minimumForceRequired}, got {impactForce}");
        }
    }

    // Enhanced collision detection method
    void OnCollisionEnter(Collision collision)
    {
        Debug.Log($"Door collision with: {collision.gameObject.name}");
        CheckForBallCollision(collision.gameObject, collision.relativeVelocity.magnitude,
                              collision.contacts.Length > 0 ? collision.contacts[0].point : transform.position);
    }

    // Detection when ball enters the trigger zone
    public void OnBallDetected(GameObject ballObject, float velocity)
    {
        Debug.Log($"Ball detected near door: {ballObject.name}, velocity: {velocity}");

        // Even if ball misses the door, if it's moving fast enough and close enough, open the door
        Vector3 hitPoint = ballObject.transform.position;
        CheckForBallCollision(ballObject, velocity, hitPoint);
    }

    void CheckForBallCollision(GameObject collisionObject, float velocity, Vector3 hitPoint)
    {
        // Check if it's the crystal ball
        ThrowableBall ball = collisionObject.GetComponent<ThrowableBall>();
        bool isCrystalBall = ball != null || collisionObject.name.Contains("Crystal") || collisionObject.name.Contains("Ball");

        if (isCrystalBall)
        {
            Debug.Log($"Crystal ball detected! Velocity: {velocity}");

            // Make it easier to open - use velocity directly as force
            float impactForce = Mathf.Max(velocity, minimumForceRequired + 1f); // Ensure it always has enough force

            OnBallHit(impactForce, hitPoint);
        }
        else
        {
            Debug.Log($"Not a crystal ball: {collisionObject.name}");
        }
    }

    IEnumerator OpenDoor()
    {
        isAnimating = true;

        // Play opening sound
        if (openSound != null)
        {
            openSound.Play();
        }

        Vector3 targetPosition = originalPosition;
        Quaternion targetRotation = originalRotation;

        if (slideInsteadOfRotate)
        {
            targetPosition = originalPosition + slideDirection.normalized * slideDistance;
        }
        else
        {
            targetRotation = originalRotation * Quaternion.Euler(0, openAngle, 0);
        }

        float elapsedTime = 0;
        Vector3 startPos = transform.position;
        Quaternion startRot = transform.rotation;

        while (elapsedTime < 1f)
        {
            elapsedTime += Time.deltaTime * openSpeed;
            float t = elapsedTime;

            // Smooth animation curve
            t = Mathf.SmoothStep(0, 1, t);

            if (slideInsteadOfRotate)
            {
                transform.position = Vector3.Lerp(startPos, targetPosition, t);
            }
            else
            {
                transform.rotation = Quaternion.Lerp(startRot, targetRotation, t);
            }

            yield return null;
        }

        // Ensure final position/rotation is exact
        if (slideInsteadOfRotate)
        {
            transform.position = targetPosition;
        }
        else
        {
            transform.rotation = targetRotation;
        }

        isOpen = true;
        isAnimating = false;

        Debug.Log("Door opened successfully!");

        // Start close timer if enabled
        if (canClose && closeDelay > 0)
        {
            if (closeCoroutine != null)
            {
                StopCoroutine(closeCoroutine);
            }
            closeCoroutine = StartCoroutine(CloseAfterDelay());
        }
    }

    IEnumerator CloseDoor()
    {
        isAnimating = true;

        // Stop close timer if running
        if (closeCoroutine != null)
        {
            StopCoroutine(closeCoroutine);
            closeCoroutine = null;
        }

        // Play closing sound
        if (closeSound != null)
        {
            closeSound.Play();
        }

        float elapsedTime = 0;
        Vector3 startPos = transform.position;
        Quaternion startRot = transform.rotation;

        while (elapsedTime < 1f)
        {
            elapsedTime += Time.deltaTime * openSpeed;
            float t = elapsedTime;

            // Smooth animation curve
            t = Mathf.SmoothStep(0, 1, t);

            if (slideInsteadOfRotate)
            {
                transform.position = Vector3.Lerp(startPos, originalPosition, t);
            }
            else
            {
                transform.rotation = Quaternion.Lerp(startRot, originalRotation, t);
            }

            yield return null;
        }

        // Ensure final position/rotation is exact
        transform.position = originalPosition;
        transform.rotation = originalRotation;

        isOpen = false;
        isAnimating = false;

        Debug.Log("Door closed!");
    }

    IEnumerator CloseAfterDelay()
    {
        yield return new WaitForSeconds(closeDelay);

        if (isOpen && !isAnimating)
        {
            StartCoroutine(CloseDoor());
        }
    }

    // Public methods for external control
    public void ForceDoorOpen()
    {
        if (!isOpen && !isAnimating)
        {
            StartCoroutine(OpenDoor());
        }
    }

    public void ForceDoorClose()
    {
        if (isOpen && !isAnimating)
        {
            StartCoroutine(CloseDoor());
        }
    }

    public void ToggleDoor()
    {
        if (isAnimating) return;

        if (isOpen)
        {
            StartCoroutine(CloseDoor());
        }
        else
        {
            StartCoroutine(OpenDoor());
        }
    }

    // Visualize the detection radius in Scene view
    void OnDrawGizmosSelected()
    {
        Gizmos.color = Color.yellow;
        Gizmos.DrawWireSphere(transform.position, detectionRadius);

        Gizmos.color = Color.red;
        Gizmos.DrawWireCube(transform.position, transform.localScale);
    }

    // Debug method to test door opening
    [System.Diagnostics.Conditional("UNITY_EDITOR")]
    void Update()
    {
        // In editor, press O to test door opening
        if (Application.isEditor && Input.GetKeyDown(KeyCode.O))
        {
            Debug.Log("Testing door open (Editor only)");
            OnBallHit(minimumForceRequired + 1, transform.position);
        }
    }
}

// Helper class for ball detection
public class BallDetectionTrigger : MonoBehaviour
{
    public DoorController doorController;

    void OnTriggerEnter(Collider other)
    {
        if (doorController == null) return;

        // Get the ball's velocity
        Rigidbody rb = other.GetComponent<Rigidbody>();
        float velocity = rb != null ? rb.velocity.magnitude : 10f; // Default to sufficient velocity

        doorController.OnBallDetected(other.gameObject, velocity);
    }
}