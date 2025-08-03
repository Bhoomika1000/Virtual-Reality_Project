using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class ThrowableBall : MonoBehaviour
{
    [Header("Throwing Settings")]
    public float throwForce = 25f;
    public float resetDelay = 3f;
    public bool enableReset = true;

    [Header("Target Settings")]
    public Transform targetDoor;
    public float trajectoryAccuracy = 0.95f; // How accurate the throw should be
    public Vector3 doorCenterOffset = Vector3.zero; // Manual adjustment for door center

    [Header("Visual Settings")]
    public float hoverDistance = 0.5f;
    public LineRenderer trajectoryLine;

    private Camera playerCamera;
    private Rigidbody ballRb;
    private Vector3 originalPosition;
    private bool isGrabbed = false;
    private bool hasBeenThrown = false;

    void Start()
    {
        playerCamera = Camera.main;
        if (playerCamera == null)
            playerCamera = FindObjectOfType<Camera>();

        ballRb = GetComponent<Rigidbody>();
        originalPosition = transform.position;

        // Setup trajectory line
        if (trajectoryLine == null)
        {
            GameObject lineObj = new GameObject("TrajectoryLine");
            lineObj.transform.SetParent(transform);
            trajectoryLine = lineObj.AddComponent<LineRenderer>();
        }

        SetupTrajectoryLine();
    }

    void SetupTrajectoryLine()
    {
        trajectoryLine.material = new Material(Shader.Find("Sprites/Default"));
        trajectoryLine.startColor = Color.green;
        trajectoryLine.endColor = Color.green;
        trajectoryLine.startWidth = 0.05f;
        trajectoryLine.endWidth = 0.05f;
        trajectoryLine.positionCount = 2;
        trajectoryLine.enabled = false;
        trajectoryLine.useWorldSpace = true;
    }

    Vector3 GetDoorTargetPosition()
    {
        if (targetDoor == null) return Vector3.zero;

        Vector3 targetPosition;

        // Try to get visual center using bounds
        Renderer doorRenderer = targetDoor.GetComponent<Renderer>();
        if (doorRenderer != null)
        {
            targetPosition = doorRenderer.bounds.center;
        }
        else
        {
            targetPosition = targetDoor.position;
        }

        // Apply manual offset for fine-tuning
        targetPosition += doorCenterOffset;

        // Debug the target position
        Debug.Log($"Door target position: {targetPosition}, Door transform: {targetDoor.position}");

        return targetPosition;
    }

    void Update()
    {
        // Only work when cursor is unlocked
        bool cursorUnlocked = Cursor.lockState == CursorLockMode.None;

        if (cursorUnlocked && !hasBeenThrown)
        {
            HandleBallInteraction();
        }
    }

    void HandleBallInteraction()
    {
        // Check for mouse input on ball
        if (Input.GetMouseButtonDown(0))
        {
            Ray ray = playerCamera.ScreenPointToRay(Input.mousePosition);
            RaycastHit hit;

            if (Physics.Raycast(ray, out hit))
            {
                if (hit.collider.gameObject == gameObject)
                {
                    GrabBall();
                }
            }
        }

        // Handle ball release
        if (isGrabbed && Input.GetMouseButtonUp(0))
        {
            ThrowBall();
        }

        // Update visual feedback while grabbed
        if (isGrabbed)
        {
            UpdateGrabbedBallVisuals();
        }
    }

    void GrabBall()
    {
        isGrabbed = true;
        ballRb.isKinematic = true;

        // Show trajectory to door
        if (targetDoor != null)
        {
            trajectoryLine.enabled = true;
            Vector3 doorCenter = GetDoorTargetPosition();
            trajectoryLine.SetPosition(0, transform.position);
            trajectoryLine.SetPosition(1, doorCenter);
        }

        Debug.Log("Crystal grabbed! Preparing precise throw trajectory...");
    }

    void UpdateGrabbedBallVisuals()
    {
        // Hover the ball slightly
        Vector3 hoverPos = originalPosition + Vector3.up * hoverDistance;
        transform.position = hoverPos;

        // Update trajectory line
        if (trajectoryLine.enabled && targetDoor != null)
        {
            Vector3 doorCenter = GetDoorTargetPosition();
            trajectoryLine.SetPosition(0, transform.position);
            trajectoryLine.SetPosition(1, doorCenter);

            // Pulse the line color for better visibility
            float pulse = Mathf.Sin(Time.time * 3f) * 0.3f + 0.7f;
            Color pulsedGreen = new Color(0, pulse, 0, 0.8f);
            trajectoryLine.startColor = pulsedGreen;
            trajectoryLine.endColor = pulsedGreen;
        }
    }

    void ThrowBall()
    {
        isGrabbed = false;
        hasBeenThrown = true;
        trajectoryLine.enabled = false;

        ballRb.isKinematic = false;
        ballRb.useGravity = true;

        if (targetDoor != null)
        {
            // Get the precise target position
            Vector3 targetPosition = GetDoorTargetPosition();

            // Calculate precise trajectory to door center
            Vector3 throwDirection = (targetPosition - transform.position).normalized;

            // Add slight trajectory arc for realism
            Vector3 arcedDirection = throwDirection + Vector3.up * 0.2f;
            arcedDirection.Normalize();

            // Apply enhanced throwing force for guaranteed reach
            Vector3 throwVelocity = arcedDirection * throwForce;
            ballRb.velocity = throwVelocity;

            Debug.Log("Executing precision throw! Ball trajectory calculated for optimal door impact.");
        }
        else
        {
            Debug.LogWarning("No target door assigned! Ball thrown with standard physics.");
            // Fallback: throw forward
            ballRb.velocity = playerCamera.transform.forward * throwForce;
        }

        // Reset ball after delay
        if (enableReset)
        {
            Invoke(nameof(ResetBall), resetDelay);
        }
    }

    void ResetBall()
    {
        Debug.Log("Resetting crystal position for next attempt...");

        ballRb.velocity = Vector3.zero;
        ballRb.angularVelocity = Vector3.zero;
        ballRb.isKinematic = true;

        transform.position = originalPosition;

        ballRb.isKinematic = false;
        hasBeenThrown = false;
    }

    void OnCollisionEnter(Collision collision)
    {
        if (hasBeenThrown && (collision.gameObject.name.Contains("door") ||
            collision.gameObject.name.Contains("gate") ||
            collision.gameObject == targetDoor?.gameObject))
        {
            float impactForce = collision.impulse.magnitude;
            Debug.Log($"Crystal impact detected! Force: {impactForce}");

            // Notify door of successful hit
            DoorController doorController = collision.gameObject.GetComponent<DoorController>();
            if (doorController != null)
            {
                doorController.OnBallHit(impactForce, collision.contacts[0].point);
                Debug.Log("Door opening mechanism triggered by crystal impact!");
            }
        }
    }

    void OnDrawGizmosSelected()
    {
        if (targetDoor != null)
        {
            Vector3 targetPos = GetDoorTargetPosition();
            Gizmos.color = Color.green;
            Gizmos.DrawLine(transform.position, targetPos);
            Gizmos.DrawWireSphere(targetPos, 0.5f);

            // Show door transform position in red
            Gizmos.color = Color.red;
            Gizmos.DrawWireSphere(targetDoor.position, 0.3f);
        }
    }
}