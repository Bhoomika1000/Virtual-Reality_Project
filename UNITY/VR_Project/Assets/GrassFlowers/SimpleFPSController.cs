using System.Collections;
using System.Collections.Generic;

using UnityEngine;

public class SimpleFPSController : MonoBehaviour
{
    [Header("Movement Settings")]
    public float walkSpeed = 5f;
    public float mouseSensitivity = 1f; // REDUCED from 2f to 1f for less dizziness

    [Header("Camera Settings")]
    public float cameraHeight = 2.5f; // Adjustable camera height - increase for higher view

    [Header("Ground Check")]
    public LayerMask groundLayer = -1;
    public float groundCheckDistance = 0.1f;

    private Camera playerCamera;
    private CharacterController controller;
    private float verticalRotation = 0f;
    private bool isGrounded;
    private float verticalVelocity = 0f; // Track vertical velocity separately
    private bool isCursorLocked = true; // Track cursor state

    void Start()
    {
        // Find the Main Camera in the scene
        playerCamera = Camera.main;
        if (playerCamera == null)
        {
            playerCamera = FindObjectOfType<Camera>();
        }

        if (playerCamera != null)
        {
            // Move player to camera's position (minus the eye height)
            Vector3 cameraPos = playerCamera.transform.position;
            transform.position = new Vector3(cameraPos.x, cameraPos.y - cameraHeight, cameraPos.z);
            transform.rotation = Quaternion.Euler(0, playerCamera.transform.eulerAngles.y, 0);

            // Make camera a child of player and set its height
            playerCamera.transform.SetParent(transform);
            playerCamera.transform.localPosition = new Vector3(0, cameraHeight, 0);
            playerCamera.transform.localRotation = Quaternion.identity;
        }

        // Get or create CharacterController
        controller = GetComponent<CharacterController>();
        if (controller == null)
        {
            controller = gameObject.AddComponent<CharacterController>();
            controller.height = cameraHeight + 0.2f; // Slightly taller than camera
            controller.radius = 0.5f;
            controller.center = new Vector3(0, controller.height / 2f, 0);
        }

        // Lock cursor to center of screen initially
        LockCursor();
    }

    void Update()
    {
        HandleCursorToggle(); // NEW: Handle cursor toggle
        HandleMovement();

        // Only handle mouse look if cursor is locked
        if (isCursorLocked)
        {
            HandleMouseLook();
        }

        CheckGrounded();
    }

    void HandleCursorToggle()
    {
        // Press ESC to toggle cursor lock/unlock
        if (Input.GetKeyDown(KeyCode.Escape))
        {
            if (isCursorLocked)
            {
                UnlockCursor();
            }
            else
            {
                LockCursor();
            }
        }
    }

    void LockCursor()
    {
        Cursor.lockState = CursorLockMode.Locked;
        Cursor.visible = false;
        isCursorLocked = true;
    }

    void UnlockCursor()
    {
        Cursor.lockState = CursorLockMode.None;
        Cursor.visible = true;
        isCursorLocked = false;
    }

    void HandleMovement()
    {
        // Get input
        float horizontal = Input.GetAxis("Horizontal"); // A/D
        float vertical = Input.GetAxis("Vertical");     // W/S

        // Calculate horizontal movement direction
        Vector3 direction = transform.right * horizontal + transform.forward * vertical;
        direction = direction.normalized;

        // Apply horizontal movement
        Vector3 movement = direction * walkSpeed * Time.deltaTime;

        // Handle vertical movement (gravity) separately
        if (isGrounded)
        {
            verticalVelocity = 0f; // Reset falling velocity when grounded
        }
        else
        {
            verticalVelocity -= 9.81f * Time.deltaTime; // Apply gravity
        }

        // Apply vertical movement
        movement.y = verticalVelocity * Time.deltaTime;

        controller.Move(movement);
    }

    void HandleMouseLook()
    {
        // Get mouse input
        float mouseX = Input.GetAxis("Mouse X") * mouseSensitivity;
        float mouseY = Input.GetAxis("Mouse Y") * mouseSensitivity;

        // Horizontal rotation (Y-axis) - rotate the player body
        transform.Rotate(0, mouseX, 0);

        // Vertical rotation (X-axis) - rotate the camera up/down
        verticalRotation -= mouseY;
        verticalRotation = Mathf.Clamp(verticalRotation, -90f, 90f);

        playerCamera.transform.localRotation = Quaternion.Euler(verticalRotation, 0, 0);
    }

    void CheckGrounded()
    {
        // Check if the bottom of the character controller is touching ground
        Vector3 spherePosition = new Vector3(transform.position.x, transform.position.y - controller.height / 2f + controller.radius, transform.position.z);
        isGrounded = Physics.CheckSphere(spherePosition, controller.radius + 0.01f, groundLayer);

        // Debug visualization
        Debug.DrawRay(transform.position, Vector3.down * (controller.height / 2f + 0.1f), isGrounded ? Color.green : Color.red);
    }

    void OnEnable()
    {
        LockCursor();
    }

    void OnDisable()
    {
        UnlockCursor();
    }

    // PUBLIC METHOD: Call this from button scripts to unlock cursor
    public void UnlockCursorForUI()
    {
        UnlockCursor();
    }
}