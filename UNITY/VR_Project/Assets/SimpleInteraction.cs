using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SimpleInteraction : MonoBehaviour
{
    [Header("Interaction Settings")]
    public string interactionType = "Button"; // Button, Crystal, Door
    public string buttonName = ""; // Lucy, Edmund, Susan, Peter (for buttons only)

    [Header("Visual Feedback")]
    public bool highlightOnLookAt = true;
    public Color highlightColor = Color.white;
    private Color originalColor;
    private Renderer objectRenderer;

    [Header("References")]
    public EscapeRoomManager escapeManager;

    [Header("Interaction Cooldown")]
    public float cooldownTime = 1f; // Prevent multiple rapid interactions
    private float lastInteractionTime = 0f;

    void Start()
    {
        // Get the renderer for highlighting
        objectRenderer = GetComponent<Renderer>();
        if (objectRenderer != null)
            originalColor = objectRenderer.material.color;

        // Find the escape room manager if not assigned
        if (escapeManager == null)
            escapeManager = FindObjectOfType<EscapeRoomManager>();

        // Add collider if missing and make it a trigger for walking interaction
        Collider col = GetComponent<Collider>();
        if (col == null)
        {
            col = gameObject.AddComponent<SphereCollider>();
        }

        // Make collider a trigger for walk-through interaction
        col.isTrigger = true;

        // Make trigger bigger for easier interaction
        if (col is SphereCollider)
        {
            ((SphereCollider)col).radius = 1.5f;
        }

        Debug.Log("Interactive object ready: " + gameObject.name + " (" + interactionType + ")");
    }

    // For mouse/touch interaction
    void OnMouseDown()
    {
        Debug.Log("OnMouseDown triggered on: " + gameObject.name + " (Type: " + interactionType + ")");
        HandleInteraction();
    }

    // For collision-based interaction (walking into objects)
    void OnTriggerEnter(Collider other)
    {
        Debug.Log("OnTriggerEnter triggered by: " + other.name + " with tag: " + other.tag + " on object: " + gameObject.name);
        if (other.CompareTag("Player"))
        {
            Debug.Log("Player tag confirmed! Calling HandleInteraction...");
            HandleInteraction();
        }
        else
        {
            Debug.Log("Not a Player tag, ignoring...");
        }
    }

    void HandleInteraction()
    {
        Debug.Log("HandleInteraction called on: " + gameObject.name + " (Type: " + interactionType + ")");

        // Cooldown to prevent multiple rapid interactions
        if (Time.time - lastInteractionTime < cooldownTime)
        {
            Debug.Log("Cooldown active, ignoring interaction");
            return;
        }
        lastInteractionTime = Time.time;

        if (escapeManager == null)
        {
            Debug.LogWarning("EscapeRoomManager not found!");
            return;
        }

        Debug.Log("About to call escape manager method for: " + interactionType);

        switch (interactionType.ToLower())
        {
            case "button":
                if (!string.IsNullOrEmpty(buttonName))
                {
                    Debug.Log("Button pressed: " + buttonName);
                    escapeManager.ButtonPressed(buttonName);
                }
                break;
            case "crystal":
                Debug.Log("Crystal touched!");
                escapeManager.CrystalTouched();
                break;
            case "door":
                Debug.Log("Door touched!");
                escapeManager.DoorTouched();
                break;
        }
    }

    // Highlight when looking at object
    void OnMouseEnter()
    {
        if (highlightOnLookAt && objectRenderer != null)
        {
            objectRenderer.material.color = highlightColor;
        }
    }

    void OnMouseExit()
    {
        if (highlightOnLookAt && objectRenderer != null)
        {
            objectRenderer.material.color = originalColor;
        }
    }
}